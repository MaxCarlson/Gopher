#pragma once

using coord = int;

constexpr int Pass = -1;
constexpr int Resign = -2;

struct Move
{
	coord idx;
	int color;

	bool operator==(const Move& other) const noexcept
	{
		return (idx == other.idx) && (color == other.color);
	}
};

inline bool isPass(const Move& m) noexcept
{
	return m.idx == Pass;
}

inline bool isResign(const Move& m) noexcept
{
	return m.idx == Resign;
}