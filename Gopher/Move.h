#pragma once

using coord = int;

constexpr int Pass = -1;
constexpr int Resign = -2;

struct Move
{
	coord idx;
	int color;
};

inline bool isPass(const Move& m)
{
	return m.idx == Pass;
}

inline bool isResign(const Move& m)
{
	return m.idx == Resign;
}