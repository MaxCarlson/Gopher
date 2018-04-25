#pragma once
#include "Stone.h"
#include "Move.h"

constexpr int BoardSize = 19;
constexpr int BoardOffset = 2;
constexpr int BoardMaxIdx = (BoardSize + BoardOffset) * (BoardSize + BoardOffset);
constexpr int BoardMaxGroups = 228;//BoardSize * BoardSize * 2 / 3;

using group = int;

struct Group
{

};

struct Board
{

	int moveCount;

	// Stones info
	int stones[BoardMaxIdx];

	
};


void printBoard(const Board& board);