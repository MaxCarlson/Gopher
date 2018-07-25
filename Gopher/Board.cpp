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
		std::cerr << " .";
		break;
	case Stone::BLACK:
		std::cerr << " b";
		break;
	case Stone::WHITE:
		std::cerr << " w";
		break;
	case Stone::OFFBOARD:
		std::cerr << " X";
		break;
	default:
		std::cerr << "BoardIdxIssue!";
	}
}

void printRow(const Board& board, int row)
{
	std::cerr << std::setw(2) << gtpFlipY(row) << "|";
	for (int i = 1; i < BoardRealSize - 1; ++i)
		printPos(board.at(getIdx(i, row)));
	std::cerr << " | \n";
}

void printBoardTop(int spaces)
{
	for (int i = 1; i < BoardRealSize - 1; ++i)
	{
		for (int j = 0; j < spaces; ++j)
			std::cerr << ' ';

		std::cerr << letters[i - 1];
	}
	std::cerr << '\n';
}

void Board::printBoard() const
{
	std::cerr << "\n   ";
	printBoardTop(1);
	for (int i = 1; i < BoardRealSize - 1; ++i)
		printRow(*this, i);
	
	std::cerr << "\n";
}

void printBoardAsGroups(const Board& board)
{
	printBoardTop(3);
	for (int i = 1; i < BoardRealSize - 1; ++i)
	{
		std::cerr << i << "|";
		for (int j = 1; j < BoardRealSize - 1; ++j)
			std::cerr << " " << std::setw(3) << std::left << board.groupAt(getIdx(j, i));
		std::cerr << " | \n";
	}
	std::cerr << '\n';
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

	komi = 7.5; // TODO: Change this based on board size!
	lastMove = { Pass, Stone::NONE };

	// TODO: Generate zobrist hash stuff
}

double Board::scoreFast() const
{
	int scores[Stone::MAX] = { 0 };

	foreachPoint([&](int idx, int color)
	{
		if (color == Stone::NONE)
		{
			if (isOnePointEye(idx, Stone::BLACK))
				color = Stone::BLACK;
			else if (isOnePointEye(idx, Stone::WHITE))
				color = Stone::WHITE;
			else
				color = Stone::NONE;
		}	
		++scores[color];
	});

	return komi + scores[Stone::WHITE] - scores[Stone::BLACK];
}

double Board::scoreReal() const // NOT DONE
{
	int scores[Stone::MAX] = { 0 };
	int ownedBy[BoardRealSize2] = { 0 };

	static constexpr int filter[] = { 0, 1, 2, 0 };

	foreachPoint([&](int idx, int color)
	{
		ownedBy[idx] = filter[color];
	});

	static constexpr int Dame = 3;

	bool update;
	do 
	{
		update = false;

		foreachFreePoint([&](coord idx)
		{
			if (ownedBy[idx] == Dame)
				return;

			int neigh[4] = { 0 };
			foreachNeighbor(idx, [&](coord idx, int color)
			{
				++neigh[ownedBy[idx]];
			});

			// Owned by both colors or neighboring a dame
			// We're also a dame
			if (neigh[Stone::BLACK] && neigh[Stone::WHITE] || neigh[Dame])
			{
				ownedBy[idx] = Dame;

				foreachNeighbor(idx, [&](coord idx, int color)
				{
					if (color == Stone::NONE)
						ownedBy[idx] = Dame;
				});
				update = true;
				return;
			}

			// If we only have neighbors of one color
			// that color owns us
			if (!ownedBy[idx]
				&& (neigh[Stone::BLACK] || neigh[Stone::WHITE]))
			{
				int owner = neigh[Stone::BLACK] 
						  ? Stone::BLACK : Stone::WHITE;

				ownedBy[idx] = owner;

				foreachNeighbor(idx, [&](coord idx, int color)
				{
					if (color == Stone::NONE && !ownedBy[idx])
						ownedBy[idx] = owner;
				});

				update = true;
			}
		});

	} while (update);

	//int dames = 0;

	foreachPoint([&](coord idx, int color)
	{
		if (color != Stone::OFFBOARD && !ownedBy[idx]) // Comment out when not debugging
			std::cerr << "Iterator error in real score \n";

		//if (ownedBy[idx] == Dame) // Not currently in use
		//	++dames;

		++scores[ownedBy[idx]];
	});

	return komi + scores[Stone::WHITE] - scores[Stone::BLACK];
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

	eachDiagonalNeighbor(idx, [&](int index, int color)
	{
		++numNeighbors[color];
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

// Doesn't prevent suicide moves
bool Board::isValid(const Move & m) const
{
	if(points[m.idx] != Stone::NONE)
		return false;

	if (!isEyeLike(m.idx, flipColor(m.color)))
		return true;

	// No ko's
	if (ko == m)
		return false;

	int playingAtari = 0;
	foreachNeighbor(m.idx, [&](coord idx, int color)
	{
		playingAtari += (groupInfoAt(idx).libs == 1);
	});

	return playingAtari;
}

bool Board::isValidNoSuicide(const Move & m) const
{
	if (at(m.idx) != Stone::NONE)
		return false;
	if (immediateLibCount(m.idx) >= 1)
		return true;
	if (isEyeLike(m.idx, flipColor(m.color)) && ko == m)
		return false;

	bool res = false;
	foreachNeighbor(m.idx, [&](int idx, int color)
	{
		if (color == flipColor(m.color)
			&& groupInfoAt(idx).libs == 1)
			res = true;

		else if (color == m.color
			&& groupInfoAt(idx).libs > 1)
			res = true;
	});

	return res;
}

int Board::immediateLibCount(coord idx) const
{
	return 4 - (neighborCount(idx, Stone::BLACK) 
			 + neighborCount(idx, Stone::WHITE) 
			 + neighborCount(idx, Stone::OFFBOARD));
}

bool Board::adjacentGroupWithLibs(coord idx, int color) const
{
	bool found = false;
	foreachNeighbor(idx, [&](coord idx, int gColor)
	{
		if (gColor == color && groupInfoAt(idx).libs > 1)
			found = true;
	});
	return found;
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
	groups.groupList.emplace_back(gid);

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
	groups.groupList.findErase(groupFrom);

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

int Board::countGroup(groupId id, int max) const
{
	int count = 0;
	foreachInGroupBreak(id, [&](coord idx, bool& stop)
	{
		++count;
		if (count == max)
			stop = true;
	});
	return count;
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

	groups.groupList.findErase(gid); // This could cause performance issues! Perft Test!

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
	{
		std::cerr << '\n';
		printBoard();
		std::cerr << "Attempting to land on a non-vacant spot!! " << m.idx << '\n';
	}

	//if (m.idx == BoardRealSize * 9 + 1) // C9 Debugging!

	// We're not playing into an opponents eye
	if (!isEyeLike(m.idx, flipColor(m.color)))
	{
		groupId group = moveNonEye(m);	 
		if (groups.isGroupCaptured(group))
		{
			// TODO: Undo this and return false?
			//std::cerr << "Self Suicide! \n"; 
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
		makeMoveGtp({ Pass, Stone::NONE });
		return Pass;
	}

	auto rng = static_cast<int>(Random::fastRandom(free.size()));
	for (int i = rng; i < free.size(); ++i)
		if (tryRandomMove(color, idx, i))
			return idx;

	for (int i = 0; i < rng; ++i)
		if (tryRandomMove(color, idx, i))
			return idx;

	// No valid moves left
	makeMoveGtp({ Pass, Stone::NONE });
	return Pass;
}

// These (handicap) moves are outside of the game stateStack
bool Board::setFixedHandicap(int count)
{
	if (count > 9)
		return false;

	static constexpr int high	= 3;
	static constexpr int low	= BoardSize - 1 - high;
	static constexpr int mid	= BoardSize / 2;

	if (count >= 2)
	{
		makeMove({ getIdx(high, high), BLACK });
		makeMove({ getIdx(low,   low), BLACK });
	}

	if (count >= 3)
		makeMove({ getIdx(high, low), BLACK });

	if (count >= 4)
		makeMove({ getIdx(low, high), BLACK });

	if (count >= 5 && count % 2)
		makeMove({ getIdx(mid, mid), BLACK });

	if (count >= 6)
	{
		makeMove({ getIdx(low, mid), BLACK });
		makeMove({ getIdx(mid, low), BLACK });
	}
	return true;
}

std::string Board::stoneStrVerts() const
{
	std::stringstream ss;

	foreachPoint([&](int idx, int stone)
	{
		if (stone == OFFBOARD || stone == NONE)
			return;
		ss << moveToString(idx) << ' ';
	});
	ss << '\n';
	return ss.str();
}
