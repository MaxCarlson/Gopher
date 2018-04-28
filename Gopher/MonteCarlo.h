#pragma once
#include "Move.h"

class Board;

class MonteCarlo
{
	const Board& board;
public:
	MonteCarlo(const Board& board) : board(board) {}

	int playRandomGame(int length, double deathRatio);

	coord genMove(int color);

};
