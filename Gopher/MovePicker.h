#pragma once

struct Board;
struct Move;
struct AmafMap;

namespace MovePicker
{
	Move pickMove(Board& board, int color);
	bool tryHeuristics(Board& board, Move& ourMove);

	bool localAtari(const Board& board, Move& move, const Move& lastMove);
	bool local2Lib(const Board& board, Move& move, const Move& lastMove);

	bool nakadeCheck(const Board& board, Move& move, const Move& theirMove);
	bool global2Libs(const Board& board, Move& move, const Move& theirMove);
	bool globalCaptures(const Board& board, Move& move, const Move& theirMove);
}