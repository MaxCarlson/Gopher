#pragma once

class Board;
struct Move;
enum Stone;

struct MovePicker
{
	Move pickMove(Board& board, Stone color);
	bool tryHeuristics(Board& board, Move& ourMove);

	bool nakadeCheck(const Board& board, Move &move, const Move& theirMove);

};

inline MovePicker movePicker;