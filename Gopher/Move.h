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
	Move(coord idx, Stone color) : idx(idx), color(color)// Just for debugging
	{
		static const char* letters = { "ABCDEFGHIJKLMNOPQRSTUVWXYZ" };
		x = letters[(idx / BoardRealSize) - 1];
		y = (idx % BoardRealSize) - 1;
	}
	// Just for debugging
	char x = 0, y = 0;

	bool operator==(const Move& other) const 
	{
		return (idx == other.idx) && (color == other.color);
	}
};

inline bool isPass(const Move& m) 
{
	return m.idx == Pass;
}

inline bool isResign(const Move& m) 
{
	return m.idx == Resign;
}

inline std::pair<coord, coord> idxToXY(int idx)
{
	return { idx % BoardRealSize, idx / BoardRealSize };
}

inline std::pair<coord, coord> moveToXY(const Move& m)
{
	return idxToXY(m.idx);
}

inline void printMove(const Move& m)
{
	static const char* letters = { "ABCDEFGHIJKLMNOPQRSTUVWXYZ" };
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