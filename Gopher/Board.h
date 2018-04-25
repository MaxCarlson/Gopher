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

struct Neighbors
{
	int n[Stone::MAX];
};

struct Board
{
	int moveCount;

	// Stones info
	int stones[BoardMaxIdx];

	int freeCount;
	// Free board positions
	coord free[BoardMaxIdx];

	// Quick lookup of all an idx's neighbors
	Neighbors neighbors[BoardMaxIdx];

	// Figure out what exactly needs to be stored here
	Move ko;

public:
	int neighborCount(coord idx, Stone color) const;
	bool isEyeLike(coord idx, Stone color) const;
	bool isValid(const Move& m) const;


	bool tryRandomMove(Stone color, coord& idx, int rng) const;
	coord playRandom(Stone color);

	friend void printBoard(const Board& board);
};


