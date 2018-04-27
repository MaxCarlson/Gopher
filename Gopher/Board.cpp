#include "Board.h"
#include "Random.h"
#include <iostream>

inline int getIdx(int x, int y)
{
	return y * BoardRealSize + x;
}

void printPos(const int i)
{
	switch (i)
	{
	case Stone::NONE:
		std::cout << "   ";
		break;
	case Stone::BLACK:
		std::cout << " b ";
		break;
	case Stone::WHITE:
		std::cout << " w ";
		break;
	case Stone::OFFBOARD:
		std::cout << " X ";
		break;
	default:
		std::cout << "BoardIdxIssue!";
	}
}

void printRow(const Board& board, int row)
{
	std::cout << "| ";
	for (int i = 0; i < BoardRealSize; ++i)
		printPos(board.points[getIdx(i, row)]);
	std::cout << " | \n";
}

void printBoard(const Board & board)
{
	std::cout << '\n';
	for (int i = 0; i < BoardRealSize; ++i)
	{
		printRow(board, i);
	}
	std::cout << '\n';
}

void Board::init()
{
	std::memset(this, 0, sizeof(Board));

	// Set offboard margin
	int topRow = BoardRealSize2 - BoardRealSize;
	for (int i = 0; i < BoardRealSize; ++i)
		points[i] = points[i + topRow] = Stone::OFFBOARD;

	for (int i = 0; i <= topRow; i += BoardRealSize)
		points[i] = points[BoardRealSize - 1 + i] = Stone::OFFBOARD;

	// Set neighbor values for all points that aren't offboard
	// And create list of free positions
	foreachPoint([&](int idx, int point)
	{
		if (point == Stone::OFFBOARD)
			return;

		neighbors[idx].increment(static_cast<Stone>(points[idx]));

		free.emplace_back(idx);
	});

	// TODO: Generate zobrist hash stuff
}

int Board::neighborCount(coord idx, Stone color) const
{
	return neighbors[idx].n[color];
}

bool Board::isEyeLike(coord idx, Stone color) const
{
	return (neighborCount(idx, color) + neighborCount(idx, Stone::OFFBOARD)) == 4;
}

bool Board::isFalseEyelike(coord idx, Stone color) const
{
	int numNeighbors[Stone::MAX] = { 0, 0, 0, 0 };

	eachDiagonalNeighbor(idx, [&](int index, int id)
	{
		++numNeighbors[at(index)];
	});

	// Do we have two enemy stones diagonally (or side of the board
	// equivalent) ?
	numNeighbors[flipColor(color)] += !!numNeighbors[Stone::OFFBOARD];
	return numNeighbors[flipColor(color)] >= 2;
}

bool Board::isOnePointEye(coord idx, Stone color) const
{
	return isEyeLike(idx, color) && !isFalseEyelike(idx, color);
}

bool Board::isValid(const Move & m) const
{
	if(points[m.idx] != Stone::NONE)
		return false;

	if (!isEyeLike(m.idx, flipColor(m.color)))
		return true;

	// No ko's
	if (ko == m)
		return false;

	return true;

	// Is this an optimization? Why not allow non-eyelike - non ko
	// moves if there's a group of points ready to be captured on the board (Atari)?
	/*
	foreach_neighbor(board, coord, {
		group_t g = group_at(board, c);
		groups_in_atari += (board_group_info(board, g).libs == 1);
	});
	return !!groups_in_atari;
	*/
}

bool Board::tryRandomMove(Stone color, coord& idx, int rng)  // TODO: Pass a playout policy instead of just looking at move validity
{
	idx = free[rng];

	Move m = { idx, color };

	if (isOnePointEye(m.idx, m.color) || !isValid(m))
		return false;

	makeMove(m);
	return true;
}

coord Board::playRandom(Stone color) 
{
	coord idx;
	if (!free.size())
	{
		// Handle no moves left
	}

	int rng = fastRandom(free.size());
	for (int i = rng; i < free.size(); ++i)
		if (tryRandomMove(color, idx, rng))
			return idx;
	
	for(int i = 0; i < rng; ++i)
		if (tryRandomMove(color, idx, rng))
			return idx;

	return 0;
}

// Pass this the idx of the first group stone
// (also the group id)
bool Board::isGroupOneStone(const groupId id)
{
	return groups.groupNextStone(id);
}

void Board::groupAddLibs(groupId groupid, coord idx)
{
	Group& g = groups.groupInfoById(groupid);
	//bool isSingleStone = isGroupOneStone(groupid);

	if (g.libs < GroupLibCount)
	{
		// Don't add a liberty if we already have it
		for (int i = 0; i < GroupLibCount; ++i)
		{
			if (g.lib[i] == idx) 
				return;
		}
		g.lib[g.libs++] = idx;
	}
}

void Board::groupFindLibs(Group & group, groupId groupid, coord idx) // TODO: If LIBERTY ISSUES ~~ Look here first
{
	// Taken from pachii
	// Hash the indexes of our liberties so we can quickly
	// check and make sure we're not double counting a liberty
	unsigned char htable[BoardRealSize / 8];
	auto gethashLib = [&](int lib) -> unsigned char { return (htable[lib >> 3] & (1 << (lib & 7))); } ;
	auto sethashLib = [&](int lib) { htable[lib >> 3] |= (1 << (lib & 7)); };

	for (int i = 0; i < GroupLibCount; ++i)
		sethashLib(group.lib[i]);

	sethashLib(idx);

	foreachInGroup(groupid, [&](coord idx)
	{
		if (group.libs >= GroupLibCount)
			return;

		foreachNeighbor(idx, [&](int id, int type)
		{
			if (at(idx) + gethashLib(idx) != Stone::NONE || group.libs >= GroupLibCount)
				return;

			sethashLib(idx);
			group.lib[group.libs++] = idx;
		});
	});
}

void Board::groupRemoveLibs(groupId groupid, coord idx)
{
	Group& g = groups.groupInfoById(groupid);
	//bool isSingleStone = isGroupOneStone(groupid);

	for (int i = 0; i < GroupLibCount; ++i)
	{
		if (g.lib[i] != idx)
			continue;

		// Replace liberty we lost
		g.lib[i] = std::move(g.lib[--g.libs]);

		if (g.libs > GroupLibRefill)
			return; // No need to find more liberties

		if (g.libs == GroupLibRefill)
			groupFindLibs(g, groupid, idx);

		// TODO: Cache group as capturable (g.libs == 1)
		// TODO: Cache group as captured   (g.libs == 0)
		return;
	}
}

void Board::addToGroup(groupId group, coord neighbor, coord newStone)
{
	groupAt(newStone) = group;
	groups.groupNextStone(newStone) = groups.groupNextStone(neighbor);
	groups.groupNextStone(neighbor) = newStone;

	foreachNeighbor(newStone, [&](int idx, int type)
	{
		if (at(idx) == Stone::NONE)
			groupAddLibs(group, idx);
	});
}

groupId Board::updateNeighbor(coord nidx, const Move& m, groupId moveGroup)
{
	auto& neighbor = neighbors[nidx];
	neighbor.increment(m.color);

	const int ncolor = at(nidx);
	const groupId ngroup = groupAt(nidx);

	if (!ngroup)
		return moveGroup;

	// Remove move idx liberty from group
	groupRemoveLibs(ngroup, m.idx);

	if (m.color == ncolor && ngroup != moveGroup)
	{
		if (!moveGroup)
		{
			moveGroup = ngroup;
			addToGroup(moveGroup, nidx, m.idx);
		}
		//else
			//mergeGroup(ngroup, nidx, m.idx);
	}

	return moveGroup;
}

void Board::moveNonEye(const Move & m)
{
	groupId g = 0;

	foreachNeighbor(m.idx, [&](int idx, int type)
	{
		g = updateNeighbor(idx, m, g);
	});

	// TODO: New group if applicable

	// TODO: Update hash

	ko = { Pass, Stone::NONE };

	++moveCount;
}

void Board::makeMove(const Move & m)
{
	// We're not playing into an opponents eye
	if (!isEyeLike(m.idx, flipColor(m.color)))
	{
		moveNonEye(m);
		// TODO: Handle group suicides by looking at our recent groups liberties
	}
	else
	{

	}

	points[m.idx] = m.color;
}

