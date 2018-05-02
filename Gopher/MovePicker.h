#pragma once

class Board;
struct Move;
enum Stone;

struct MovePicker
{
	Move pickMove(Board& board, Stone color);
	bool tryHeuristics(Board& board, Move& move);

	bool nakadeCheck(const Board& board, Move& newIdx);

};

inline MovePicker movePicker;