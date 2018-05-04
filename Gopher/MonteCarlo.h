#pragma once
#include "Move.h"

struct Board;

class MonteCarlo
{
	const Board& board;
public:
	MonteCarlo(const Board& board) : board(board) {}

	coord genMove(int color);

};
