#pragma once

struct Board;
struct Move;
struct AmafMap;
enum Stone;

namespace MovePicker
{
	Move pickMove(Board& board, int color);
	bool tryHeuristics(Board& board, Move& ourMove);

	bool atariCheck(const Board& board, Move& move, const Move& lastMove);
	bool localAtari(const Board& board, Move& move, const Move& lastMove);

	bool nakadeCheck(const Board& board, Move& move, const Move& theirMove);
	bool captureCheck(const Board& board, Move& move, const Move& theirMove);
}