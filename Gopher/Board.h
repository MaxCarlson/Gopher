#pragma once
#include "Stone.h"
#include "Move.h"

constexpr int BoardSize = 19;
constexpr int BoardRealSize = BoardSize + 2;
constexpr int BoardRealSize2 = BoardRealSize * BoardRealSize;
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

template<int size>
class FastArray
{

};

struct Board
{
	int moveCount;

	// Stones info
	int points[BoardMaxIdx];

	int freeCount;
	// Free board positions
	coord free[BoardMaxIdx];

	// Quick lookup of all an idx's neighbors
	Neighbors neighbors[BoardMaxIdx];

	// Figure out what exactly needs to be stored here
	Move ko;

public:
	void init();

	int neighborCount(coord idx, Stone color) const;
	bool isEyeLike(coord idx, Stone color) const;
	bool isValid(const Move& m) const;


	bool tryRandomMove(Stone color, coord& idx, int rng) const;
	coord playRandom(Stone color);

	friend void printBoard(const Board& board);

	template<class F>
	void foreachPoint(F&& f);

	template<class F>
	void allValidSpots(F&& f);
};

template<class F>
inline void Board::foreachPoint(F&& f)
{
	for (int i = 0; i < BoardMaxIdx; ++i)
		f(points[i]);
}


