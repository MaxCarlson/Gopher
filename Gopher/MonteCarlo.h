#pragma once
#include "Move.h"

struct Board;

class MonteCarlo
{
	const Board& board;
public:
	MonteCarlo(const Board& board) : board(board) {}

	int playRandomGame(Board& board, int color, int length, double deathRatio);

	coord genMove(int color);

};
