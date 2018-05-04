#pragma once

struct Board;
struct Move;
enum Stone;

namespace MovePicker
{
	Move pickMove(Board& board, int color);
	bool tryHeuristics(Board& board, Move& ourMove);

	bool nakadeCheck(const Board& board, Move &move, const Move& theirMove);
}