#include "MovePicker.h"
#include "Board.h"
#include "Random.h"
#include <array>

constexpr int MAX_ROLL = 100;
namespace TryValues
{
	int local_atari = MAX_ROLL;
	int local_2Lib = 60;
	int nakade = 80;
	int capture = 40;
};

namespace MovePicker
{

// Goes through a list of heuristics to find a possible move
// if no heuristic hit we'll just generate a random one
Move pickMove(Board & board, int color)
{
	Move m = { Pass, color };
	if (!board.free.size())
	{
		// Handle no moves left
		board.makeMoveGtp(m);
		return m;
	}

	/*
	int searched = 0;
	int found = 0;
	board.foreachGroup([&](groupId gid)
	{
		for (int i = 0; i < BoardRealSize2; ++i)
			if (board.groups.groupIds[i] == gid)
			{
				++found;
				break;
			}

		++searched;
	});
	if (found != searched)
		std::cerr << "Oh No! " << searched - found << '\n';
		*/

	if (tryHeuristics(board, m))
	{
		board.makeMove(m);
		return m;
	}

	int rng = Random::fastRandom(board.free.size());
	for (int i = rng; i < board.free.size(); ++i)
		if (board.tryRandomMove(static_cast<Stone>(color), m.idx, i))
			return m;

	for (int i = 0; i < rng; ++i)
		if (board.tryRandomMove(static_cast<Stone>(color), m.idx, i))
			return m;

	m = { Pass, color };
	board.makeMoveGtp(m);

	return m;
}

bool tryHeuristics(Board & board, Move& ourMove)
{
	if (isPass(board.lastMove))
		return false;

	// Following Pachi's idea of local checks first

	// Check to see if opponent played self atari, 
	// or if he put one of our groups into atari
	if (TryValues::local_atari > Random::fastRandom(MAX_ROLL)
		&& localAtari(board, ourMove, board.lastMove))
		return true;

	// See if we can reduce the group our opponent played on
	// to atari
	if (TryValues::local_2Lib > Random::fastRandom(MAX_ROLL)
		&& local2Lib(board, ourMove, board.lastMove))
		return true;

	// Nakade check
	if (TryValues::nakade > Random::fastRandom(MAX_ROLL)
		&& board.immediateLibCount(board.lastMove.idx) > 0
		&& nakadeCheck(board, ourMove, board.lastMove))
		return true;
	
	// Check board for capturable enemies
	// TODO: This is Super slow
	if (TryValues::capture > Random::fastRandom(MAX_ROLL)
		&& captureCheck(board, ourMove, board.lastMove))
		return true;

	return false;
}

// Did our opponent play a self-atari move?
// Did our opponent put us into atari?
bool localAtari(const Board & board, Move & move, const Move & lastMove)
{
	coord atariIdx[2] = { 0 };
	board.foreachNeighbor(lastMove.idx, [&](coord idx, int color)
	{
		if (board.groupInfoAt(idx).libs > 1)
			return;

		atariIdx[color == move.color] = idx;
	});

	if (!atariIdx[0] && !atariIdx[1])
		return false;

	const auto findLiberty = [&](coord atariAt)
	{
		const groupId atariGid = board.groupAt(atariAt);

		bool savingMove = false;
		const auto& group = board.groupInfoAt(atariAt);

		group.forCachedLibs([&](coord idx)
		{
			// Pick a saving move that's not a suicide
			if (board.at(idx) == Stone::NONE 
				&& (board.immediateLibCount(idx) > 0 
				||  board.adjacentGroupWithLibs(idx, move.color)))
			{
				move.idx = idx;
				savingMove = true;
			}
		});

		return savingMove;
	};

	// Capture other group if possible
	// TODO: Do a random roll here and don't capture
	// if it's a small group, could be a better move out there?
	if (atariIdx[1] && findLiberty(atariIdx[1]))
		return true;

	// Save our group
	// This can and does lead to playing futile attampts
	// to keep a dead group alive
	if (atariIdx[0] && findLiberty(atariIdx[0]))
		return true;

	return false;
}

bool local2Lib(const Board & board, Move & move, const Move & lastMove)
{
	groupId gid = board.groupAt(lastMove.idx);

	if (board.groupLibCount(gid) > 2)
		return false;

	bool found = false;

	const auto& group = board.groupInfoAt(lastMove.idx);

	group.forCachedLibs([&](coord idx)
	{
		// Pick a move that reduces the groups liberties to 1 (Atari)
		if (board.at(idx) == Stone::NONE 
			&& (board.immediateLibCount(idx) > 0 
			||  board.adjacentGroupWithLibs(idx, move.color)))
		{
			move.idx = idx;
			found = true;
		}
	});

	return found;
}

bool captureCheck(const Board & board, Move & move, const Move & theirMove)
{
	// TODO: Cache groups so we don't have to loop through every point
	// This would allow for randomly iterating through groups easier as well
	//
	// TODO: Look at moves to reduce liberty counts to atari as well?

	FastList<groupId, BoardMaxGroups> groupsInAtari;

	board.foreachGroup([&](groupId gid)
	{
		// Don't look at our groups
		if (board.at(gid) != theirMove.color)
			return;

		const auto& group = board.groups.groupInfoById(gid);
		if (group.libs > 1)
			return;

		groupsInAtari.emplace_back(gid);
	});

	if (!groupsInAtari.size())
		return false;

	const auto findCapture = [&](groupId gid) -> bool
	{
		const auto& group = board.groupInfoGid(gid);
		
		bool found = false; // TODO: Found probably isn't nescessary since
		group.forCachedLibs([&](coord idx)
		{
			found = true;
			move.idx = idx;
		});
		return found;
	};

	// Select a random group in atari
	const auto rng = Random::fastRandom(groupsInAtari.size());

	for (int i = rng; i < groupsInAtari.size(); ++i)
		if (findCapture(groupsInAtari[i]))
			return true;
			
	for (int i = 0; i < rng; ++i)
		if (findCapture(groupsInAtari[i]))
			return true;

	return false;
}

bool nakadeCheck(const Board& board, Move &move, const Move& theirMove)
{
	// This seems like it wouldn't work well for nakade,
	// TODO: Figure out how is8adjacent makes sense
	coord adj = Pass;
	board.foreachNeighbor(theirMove.idx, [&](coord idx, int color)
	{
		if (color != Stone::NONE)
			return;
		if (adj == Pass)
		{
			adj = idx;
			return;
		}
		if (!is8Adjacent(adj, idx))
			adj = Resign;
	});

	if (isPass(adj) || isResign(adj))
		return false;

	static constexpr int MAX_NAKADE_SIZE = 6;
	Stone ourColor = flipColor(theirMove.color);

	int nSize = 0;
	bool impossible = false;
	coord nakadeArea[MAX_NAKADE_SIZE];
	nakadeArea[nSize++] = adj;

	// Create an array of the inside points of the Nakade shape
	// if it's too big it isn't Nakade
	for (int i = 0; i < nSize; ++i)
	{
		board.foreachNeighbor(nakadeArea[i], [&](int idx, int color)
		{
			if (color == ourColor)
				impossible = true;

			if (color == Stone::NONE)
			{
				// Don't count duplicates
				for (int j = 0; j < nSize; ++j)
					if (nakadeArea[j] == idx)
						return;

				if (nSize >= MAX_NAKADE_SIZE)
				{
					impossible = true;
					return;
				}
				nakadeArea[nSize++] = idx;
			}
		});
		if (impossible)
			return false;
	}

//board.printBoard();

	// Holds neighbor count of nakadeArea[i]
	int neighbors[MAX_NAKADE_SIZE] = { 0 };

	// Holds a count of how many neighbors have [idx] number points
	int neighCount[MAX_NAKADE_SIZE] = { nSize, 0 };

	for(int i = 0; i < nSize; ++i)
		for (int j = i + 1; j < nSize; ++j)
			if (isAdjacent(nakadeArea[i], nakadeArea[j]))
			{
				--neighCount[neighbors[i]];
				++neighbors[i];
				++neighCount[neighbors[i]];
				--neighCount[neighbors[j]];
				++neighbors[j];
				++neighCount[neighbors[j]];
			}

	// Build a list of points indexed by their neighbor count
	int idxByCount[MAX_NAKADE_SIZE];
	for (int i = 0; i < nSize; ++i)
		idxByCount[neighbors[i]] = std::move(nakadeArea[i]);
		
	switch (nSize)
	{
	case 1:
	case 2:
		return false;
	case 3:
		if (neighCount[2] != 1) // JUST FOR DEBUGGING, REMOVE LATER
			std::cout << "Nakade Issue!! \n";

		move.idx = idxByCount[2]; // Middle of the three
		return true;
	case 4:
		if (neighCount[3] != 1)
			return false;
		move.idx = idxByCount[3]; // Pyramid four
		return true;
	case 5:
		if (neighCount[3] == 1 && neighCount[1] == 1)
			move.idx = idxByCount[3]; // Bulky five
		else if (neighCount[4] == 1)
			move.idx = idxByCount[4]; // Crossed five
		else
			return false;
		return true;
	case 6:
		if (neighCount[4] == 1 && neighCount[2] == 3)
			move.idx = idxByCount[4]; // Rabbity six
		else
			return false;
		return true;

	default:
		std::cout << "Nakade error! This should be unreachable! \n";
	}

	return false;
}

}// End MovePicker::
