#include "Board.h"
#include "Random.h"
#include "Gtp.h"
#include <iostream>
#include <iomanip>

inline int getIdx(int x, int y)
{
	return y * BoardRealSize + x;
}

void printPos(const int i)
{
	switch (i)
	{
	case Stone::NONE:
		std::cout << " .";
		break;
	case Stone::BLACK:
		std::cout << " b";
		break;
	case Stone::WHITE:
		std::cout << " w";
		break;
	case Stone::OFFBOARD:
		std::cout << " X";
		break;
	default:
		std::cout << "BoardIdxIssue!";
	}
}

void printRow(const Board& board, int row)
{
	std::cout << std::setw(2) << gtpFlipY(row) << "|";
	for (int i = 1; i < BoardRealSize - 1; ++i)
		printPos(board.at(getIdx(i, row)));
	std::cout << " | \n";
}

void printBoardTop(int spaces)
{
	for (int i = 1; i < BoardRealSize - 1; ++i)
	{
		for (int j = 0; j < spaces; ++j)
			std::cout << ' ';

		std::cout << letters[i - 1];
	}
	std::cout << '\n';
}

void Board::printBoard() const
{
	std::cout << "\n   ";
	printBoardTop(1);
	for (int i = 1; i < BoardRealSize - 1; ++i)
		printRow(*this, i);
	
	std::cout << "\n";
}

void printBoardAsGroups(const Board& board)
{
	printBoardTop(3);
	for (int i = 1; i < BoardRealSize - 1; ++i)
	{
		std::cout << i << "|";
		for (int j = 1; j < BoardRealSize - 1; ++j)
			std::cout << " " << std::setw(3) << std::left << board.groupAt(getIdx(j, i));
		std::cout << " | \n";
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

		foreachNeighbor(idx, [&](int nidx, int type)
		{	// No need to handle other types since this is default init
			if (type == Stone::OFFBOARD)
				neighbors[idx].increment(Stone::OFFBOARD);
		});

		free.emplace_back(idx);
	});

	komi = 6.5; // TODO: Change this based on board size!
	// TODO: Generate zobrist hash stuff
}

double Board::scoreFast() const
{
	int scores[Stone::MAX] = { 0 };

	foreachPoint([&](int idx, int type)
	{
		if (type == Stone::NONE)
		{
			if (isOnePointEye(idx, Stone::BLACK))
				type = Stone::BLACK;
			else if (isOnePointEye(idx, Stone::WHITE))
				type = Stone::WHITE;
			else
				type = Stone::NONE;
		}	
		++scores[type];
	});

	return komi + scores[Stone::WHITE] - scores[Stone::BLACK];
}

double Board::scoreReal(const MoveStack &moves) const
{
	int ownedBy[BoardMaxIdx] = { 0 };

	int scores[Stone::MAX] = { 0 };
	const int zeroOut[] = { 0, 1, 2, 0 };

	// Add up current points
	foreachPoint([&](int idx, int color)
	{
		ownedBy[idx] = zeroOut[color];
		++scores[color];
	});

	// Look at dead groups
	for (int i = 0; i < moves.size(); ++i)
	{
		foreachInGroup(moves[i].idx, [&](groupId id)
		{

		});
	}

	return 0.0;
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
		for (int i = 0; i < g.libs; ++i)
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
	unsigned char htable[BoardMaxIdx / 8];
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

	for (int i = 0; i < g.libs; ++i)
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

groupId Board::newGroup(coord idx)
{
	groupId gid = idx;
	Group& group = groups.groupInfoById(gid);

	foreachNeighbor(idx, [&](int idx, int type)
	{
		if (at(idx) == Stone::NONE)
			group.addLib(idx);
	});

	groupAt(idx) = gid;
	groups.groupNextStone(idx) = 0;

	return gid;
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

void Board::mergeGroups(groupId groupTo, groupId groupFrom)
{
	Group& gto = groups.groupInfoById(groupTo);
	Group& gfrom = groups.groupInfoById(groupFrom);

	//bool oneStoneTo = isGroupOneStone(groupTo);
	//bool oneStoneFrom = isGroupOneStone(groupFrom);

	if (gto.libs < GroupLibCount)
	{
		for (int i = 0; i < gfrom.libs; ++i)
		{
			bool cont = false;
			for (int j = 0; j < gto.libs; ++j)
				if (gto.lib[j] == gfrom.lib[i])
				{
					cont = true;
					break;
				}
			if (cont)
				continue;

			// TODO: Use oppertunity to cache possible captureable groups (gto.libs == 0)
			// TODO: Use oppertunity to remove possible captureable groups (gto.libs == 1)

			gto.lib[gto.libs++] = std::move(gfrom.lib[i]);
			if (gto.libs >= GroupLibCount)
				break;
		}
	}

	// Set all stones in from group 
	// to belong to groupTo
	coord lastInGroup;
	foreachInGroup(groupFrom, [&](coord idx)
	{
		lastInGroup = idx;
		groups.groupIds[idx] = groupTo;
	});

	// Connect the group stons
	groups.groupNextStone(lastInGroup) = groups.groupNextStone(groupTo);
	groups.groupNextStone(groupTo) = groupFrom;

	std::memset(&gfrom, 0, sizeof(Group));
}

void Board::removeStone(groupId gid, coord idx)
{
	Stone color = static_cast<Stone>(at(idx));
	at(idx) = Stone::NONE;
	groups.groupIds[idx] = 0;

	// Increase liberties of surrounding groups
	coord stoneIdx = idx;
	foreachNeighbor(idx, [&](int idx, int type)
	{
		neighbors[idx].decrement(color);
		groupId g = groups.groupIds[idx];

		if (g && g != gid)
			groupAddLibs(g, stoneIdx);
	});

	// TODO: Update board hash

	// Add free spot
	free.emplace_back(idx);
}

int Board::groupCapture(groupId gid)
{
	int stoneCount = 0;

	foreachInGroup(gid, [&](int idx)
	{
		++stoneCount;
		++captures[flipColor(at(idx))];
		removeStone(gid, idx);
	});

	Group& g = groups.groupInfoById(gid);

	if (g.libs != 0)
		std::cout << "Non zero libs group caputure! " << g.libs << "\n";

	std::memset(&g, 0, sizeof(Group));

	return stoneCount;
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

	// Is the moving piece the same color as this neighbor 
	// and not in our group?
	if (m.color == ncolor && ngroup != moveGroup)
	{
		// Not part of a group? Lets create one
		if (!moveGroup)
		{
			moveGroup = ngroup;
			addToGroup(moveGroup, nidx, m.idx);
		}
		else
			mergeGroups(moveGroup, ngroup);
	}
	// Piece is opposite color, 
	// Check for captures
	else if (ncolor == flipColor(m.color))
	{
		if (groups.isGroupCaptured(ngroup))
			groupCapture(ngroup);
	}
	return moveGroup;
}

groupId Board::moveNonEye(const Move & m)
{
	groupId g = 0;

	foreachNeighbor(m.idx, [&](int idx, int type)
	{
		g = updateNeighbor(idx, m, g);
	});

	at(m.idx) = m.color;

	if (!g)
		g = newGroup(m.idx);

	// TODO: Update hash

	ko = { Pass, Stone::NONE };

	++moveCount;
	return g;
}

bool Board::moveInEye(const Move & m)
{
	if (m == ko)
		return false;

	// Count groups with 0 liberties after move is made
	int capturedGroups = 0;
	foreachNeighbor(m.idx, [&](int idx, int type)
	{
		groupId g = groupAt(idx);
		capturedGroups += (groups.groupInfoById(g).libs == 1);
	});

	// This is a suicide, let's not do this!
	if (capturedGroups == 0)
		return false;

	// From here on we can't fail

	int koCaps = 0;
	coord capAt = Pass;
	foreachNeighbor(m.idx, [&](int idx, int type)
	{
		neighbors[idx].increment(m.color);

		groupId gid = groupAt(idx);
		if (!gid)
			return;

		groupRemoveLibs(gid, m.idx);

		if (groups.isGroupCaptured(gid))
		{
			koCaps += groupCapture(gid);
			capAt = idx;
		}
	});

	Move newKo = { Pass, Stone::NONE };

	if (koCaps == 1)
		newKo = { capAt, flipColor(m.color) };

	at(m.idx) = m.color;
	groupId newGid = newGroup(m.idx);

	ko = std::move(newKo);
	++moveCount;

	// TODO: Update board hash
	// TODO: Cache move history for undo's

	return newGid;
}

bool Board::makeMove(const Move & m)
{
	if (at(m.idx) != Stone::NONE) // Remove when done debugging
		std::cout << "Attempting to land on a non-vacant spot!! " << m.idx << '\n';

	//if (m.idx == BoardRealSize * 9 + 1) // C9 Debugging!

	// We're not playing into an opponents eye
	if (!isEyeLike(m.idx, flipColor(m.color)))
	{
		groupId group = moveNonEye(m);	 
		if (groups.isGroupCaptured(group))
		{
			// TODO: Undo this and return false?
			//std::cout << "Self Suicide! \n"; 
			groupCapture(group);
		}
		return true;
	}
	else
		return moveInEye(m);
}

bool Board::makeMoveGtp(const Move& m)
{
	if (isPass(m) || isResign(m))
	{
		++captures[flipColor(m.color)];

		ko = { Pass, Stone::NONE };
		lastMove = m;
		return false;
	}

	bool made = makeMove(m);

	// Remove the move from our free list!
	if (made)
		free.findErase(m.idx);

	return made;
}

bool Board::tryRandomMove(Stone color, coord& idx, int freeIdx)  // TODO: Pass a playout policy instead of just looking at move validity
{
	idx = free[freeIdx];
	Move m = { idx, color };

	if (isOnePointEye(m.idx, m.color) || !isValid(m))
		return false;

	bool made = makeMove(m);

	// Remove the move we made from the list of free moves
	if (made)
	{
		free.erase(freeIdx);
		lastMove = m;
	}

	return made;
}

coord Board::playRandom(Stone color)
{
	coord idx;
	if (!free.size())
	{
		// Handle no moves left
		return Pass;
	}

	auto rng = Random::fastRandom(free.size());
	for (int i = rng; i < free.size(); ++i)
		if (tryRandomMove(color, idx, i))
			return idx;

	for (int i = 0; i < rng; ++i)
		if (tryRandomMove(color, idx, i))
			return idx;

	// No valid moves left
	return Pass;
}

