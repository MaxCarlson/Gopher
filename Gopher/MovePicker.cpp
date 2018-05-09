#include "MovePicker.h"
#include "Board.h"
#include "Random.h"

constexpr int MAX_ROLL = 100;
namespace TryValues
{
	int atari = 90;
	int nakade = 80;
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

	if (tryHeuristics(board, m))
		return m;
	
	int rng = Random::fastRandom(board.free.size());
	for (int i = rng; i < board.free.size(); ++i)
		if (board.tryRandomMove(static_cast<Stone>(color), m.idx, i))
			return m;

	for (int i = 0; i < rng; ++i)
		if (board.tryRandomMove(static_cast<Stone>(color), m.idx, i))
			return m;


	return { Pass, color };
}

bool tryHeuristics(Board & board, Move& ourMove)
{
	if (isPass(board.lastMove))
		return false;

	// Atari check
	if (TryValues::atari > Random::fastRandom(MAX_ROLL) 
		&& atariCheck(board, ourMove, board.lastMove))
		return true;

	// Nakade check
	if (TryValues::nakade > Random::fastRandom(MAX_ROLL)
		&& board.immediateLibCount(board.lastMove.idx) > 0
		&& nakadeCheck(board, ourMove, board.lastMove))
	{
		return true;
	}

	return false;
}

bool atariCheck(const Board & board, Move & move, const Move & lastMove)
{
	// Check to see if opponent move put any 
	// of our stones into atari
	coord atariIdx = 0;
	board.foreachNeighbor(lastMove.idx, [&](coord idx, int color)
	{
		if (color != move.color 
			|| board.groupInfoAt(idx).libs > 1)
			return;

		atariIdx = idx;
	});

	if (!atariIdx)
		return false;

	board.printBoard();
	printMove(lastMove);

	groupId atariGid = board.groupAt(atariIdx);

	// If so, play a saving move at random if possible
	// TODO:
	// Make this more descriminate
	
	bool savingMove = false;
	board.foreachInGroup(atariGid, [&](coord idx) 
	{
		if (board.immediateLibCount(idx) > 0)
			board.foreachNeighbor(idx, [&](coord nidx, int color)
			{
				// Pick a saving move that's not a suicide
				if (color == Stone::NONE && board.immediateLibCount(nidx) > 0)
				{
					move.idx = nidx;
					savingMove = true;
				}
			});
	});

	return savingMove;
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
