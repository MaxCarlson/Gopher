#pragma once

struct Board;
struct Move;
enum Stone;

namespace MovePicker
{
	Move pickMove(Board& board, int color);
	bool tryHeuristics(Board& board, Move& ourMove);

	bool atariCheck(const Board& board, Move &move, const Move& lastMove);
	bool nakadeCheck(const Board& board, Move &move, const Move& theirMove);
	bool captureCheck(const Board& board, Move &move, const Move& theirMove);
}