#pragma once
#include "Stone.h"
#include "Move.h"

constexpr int BoardSize = 19;
constexpr int BoardRealSize = BoardSize + 2;
constexpr int BoardRealSize2 = BoardRealSize * BoardRealSize;
constexpr int BoardOffset = 2;
constexpr int BoardMaxIdx = (BoardSize + BoardOffset) * (BoardSize + BoardOffset);
constexpr int BoardMaxGroups = 228;//BoardSize * BoardSize * 2 / 3;


template<class Type, int size>
class FastArray
{

	int mySize = 0;
	Type items[size]; // TODO: use aligned storage!
public:

	Type & operator[](const int idx)
	{
		return items[idx];
	}

	const Type & operator[](const int idx) const
	{
		return items[idx];
	}

	template<class... Args>
	void emplace_back(Args&& ...args)
	{
		new (&items[mySize]) Type(std::forward<Args>(args)...);
		++mySize;
	}

	template<class Func>
	void foreach(Func&& func)
	{
		for (int i = 0; i < mySize; ++i)
			func(items[i]);
	}

	int size() const
	{
		return mySize;
	}
};

using group = int;

struct Group
{

};

struct Neighbors
{
	int n[Stone::MAX];

	void increment(Stone type) { ++n[type]; }
};

struct Board
{
	int moveCount;

	// Capture info for all ston types
	int captures[Stone::MAX];

	// Info about squares (points) on the board
	int points[BoardMaxIdx];

	FastArray<coord, BoardMaxIdx> free;

	// Number of neighbors a stone at idx has of each type
	Neighbors neighbors[BoardMaxIdx];

	// Figure out what exactly needs to be stored here
	Move ko;

public:
	void init();

	int neighborCount(coord idx, Stone color) const;
	bool isEyeLike(coord idx, Stone color) const;
	bool isFalseEyelike(coord idx, Stone color) const;
	bool isOnePointEye(coord idx, Stone color) const;
	bool isValid(const Move& m) const;

	bool tryRandomMove(Stone color, coord& idx, int rng);
	coord playRandom(Stone color);

	// Update functions for things that happen with moves
	void updateNeighbor(coord idx, const Move& m);

	// Move functions
	void moveNonEye(const Move& m);
	void makeMove(const Move& m);

	friend void printBoard(const Board& board);

	inline int at(int idx) { return points[idx]; }

	template<class F>
	void foreachPoint(F&& f);

	template<class F>
	void foreachNeighbor(int idx, F&& f);

	template<class F>
	void eachDiagonalNeighbor(int idx, F&& f);

	template<class F>
	void eachDiagonalNeighbor(int idx, F&& f) const { eachDiagonalNeighbor(idx, f); };
};

template<class F>
inline void Board::foreachPoint(F&& f)
{
	for (int i = 0; i < BoardMaxIdx; ++i)
		f(points[i]);
}

// Don't call of points that are offboard
template<class F>
inline void Board::foreachNeighbor(int idx, F&& f)
{
	f(points[idx - 1]);
	f(points[idx + 1]);
	f(points[idx - BoardRealSize]);
	f(points[idx + BoardRealSize]);
}

template<class F>
inline void Board::eachDiagonalNeighbor(int idx, F&& f)
{
	f(points[idx + BoardRealSize + 1]);
	f(points[idx + BoardRealSize - 1]);
	f(points[idx - BoardRealSize + 1]);
	f(points[idx - BoardRealSize - 1]);
}


