#include "MovePicker.h"
#include "Board.h"
#include "Random.h"

constexpr int MAX_ROLL = 100;
namespace TryValues
{
	int nakade = 80;
};


// Goes through a list of heuristics to find a possible move
// if no heuristic hit we'll just generate a random one
Move MovePicker::pickMove(Board & board, Stone color)
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
	
	auto rng = Random::fastRandom(board.free.size());
	for (int i = rng; i < board.free.size(); ++i)
		if (board.tryRandomMove(color, m.idx, i))
			return m;

	for (int i = 0; i < rng; ++i)
		if (board.tryRandomMove(color, m.idx, i))
			return m;


	return { Pass, color };
}

bool MovePicker::tryHeuristics(Board & board, Move & move)
{
	// Nakade check
	if (TryValues::nakade > Random::fastRandom(MAX_ROLL)
		&& board.immediateLibCount(board.lastMove.idx) > 0
		&& nakadeCheck(board, move))
	{
		return true;
	}

	return false;
}

bool MovePicker::nakadeCheck(const Board& board, Move &move)
{
	// This seems like it wouldn't work well for nakade,
	// TODO: Figure out how is8adjacent makes sense
	coord adj = Pass;
	board.foreachNeighbor(move.idx, [&](coord idx, int color)
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
	Stone ourColor = flipColor(move.color);

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

				if (++nSize >= MAX_NAKADE_SIZE)
				{
					impossible = true;
					return;
				}

				nakadeArea[nSize] = idx;
			}
		});

		if (impossible)
			return false;
	}

	// Now build a list of which neighbors have other 
	// empty neighbors and how many
	int neighCount[MAX_NAKADE_SIZE];
}

