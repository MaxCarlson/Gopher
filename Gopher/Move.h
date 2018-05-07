#pragma once
#include "Stone.h"
#include "defines.h"
#include <iostream>
#include <sstream>

using coord = int;

constexpr int Pass = -1;
constexpr int Resign = -2;

struct Board;

struct Move
{
	coord idx;
	Stone color;

	Move() = default;
	Move(coord idx, int color) : idx(idx), color(static_cast<Stone>(color)) {}

	bool operator==(const Move& other) const 
	{
		return (idx == other.idx) && (color == other.color);
	}
};

inline bool isPass(const Move& m) 
{
	return m.idx == Pass;
}

inline bool isPass(coord idx)
{
	return idx == Pass;
}

inline bool isResign(const Move& m) 
{
	return m.idx == Resign;
}

inline bool isResign(coord idx)
{
	return idx == Resign;
}

inline int getX(coord idx)
{
	return idx % BoardRealSize;
}

inline int getY(coord idx)
{
	return idx / BoardRealSize;
}

inline std::pair<coord, coord> idxToXY(int idx)
{
	return { getX(idx), getY(idx) };
}

inline std::pair<coord, coord> moveToXY(const Move& m)
{
	return idxToXY(m.idx);
}

inline int xyToIdx(int x, int y)
{
	return y * BoardRealSize + x;
}

inline std::vector<int> fillFlipY()
{
	std::vector<int> flips;

	for (int i = BoardSize + 1; i > 0; --i)
		flips.emplace_back(i);
	return flips;
}

inline std::string moveToString(const Move& m)
{
	static const std::string pass = "Pass";
	static const std::string resign = "Resign";

	static const std::vector<int> flipY = fillFlipY();

	std::stringstream ss;

	switch (m.idx)
	{
	case Resign:
		return resign;
	case Pass:
		return pass;
	default:
		int x = m.idx % BoardRealSize;
		int y = m.idx / BoardRealSize;

		ss << printStone(m.color) << " ";
		ss << letters[x - 1] << flipY[y];
	}

	return ss.str();
}

inline void printMove(const Move& m)
{
	std::cout << moveToString(m) << '\n';
}