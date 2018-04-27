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

inline void printMove(const Move& m)
{
	const char* letters = { "ABCDEFGHIJKLMNOPQRSTUVWXYZ" };
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