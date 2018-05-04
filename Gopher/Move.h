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

inline void printMove(const Move& m)
{
	const std::string pass = "Pass";
	const std::string resign = "Resign";
	const auto printIdx = [&](int idx) {
		switch (idx)
		{
		case Resign:
			return resign;
		case Pass:
			return pass;
		default:
			int x = m.idx % BoardRealSize;
			int y = m.idx / BoardRealSize;

			std::stringstream ss;
			ss << letters[x - 1] << " " << y;
			return ss.str();
		}
	};

	std::cout << "Move idx: " << printIdx(m.idx) << ", Color: " << printStone(m.color) << '\n';
}