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
		&& nakadeCheck(board, move.idx))
	{
		return true;
	}

	return false;
}

bool MovePicker::nakadeCheck(const Board& board, coord& moveIdx)
{

}

