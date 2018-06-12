#pragma once
#include <vector>
#include <chrono>
#include <iostream>

constexpr int BoardSize = 19;
constexpr int BoardSize2 = BoardSize * BoardSize;
constexpr int BoardOffset = 2;
constexpr int BoardRealSize = BoardSize + BoardOffset;
constexpr int BoardRealSize2 = BoardRealSize * BoardRealSize;
constexpr int BoardMaxIdx = (BoardSize + BoardOffset) * (BoardSize + BoardOffset);
constexpr int BoardMaxGroups = BoardSize * BoardSize * 2 / 3;

constexpr int MAX_GAME_LENGTH = 400;

// No letter I in go coordinates
static const char* letters = { "ABCDEFGHJKLMNOPQRSTUVWXYZ" };

using coord = int;

// We need to reverse the y axis numbers
// e.g. on a 19x19 board 1 becomes 19, 2 becomes 18
inline int gtpFlipY(int y)
{
	static auto createFlipper = []()
	{
		std::vector<int> vec;

		for (int i = BoardSize + 1; i > 0; --i)
			vec.emplace_back(i);

		return vec;
	};

	static const auto flipper = createFlipper();

	return flipper[y];
}

namespace Time
{
	using namespace std::chrono;

	inline std::chrono::time_point<std::chrono::steady_clock> startTimer()
	{
		return high_resolution_clock::now();
	}

	template<class TimeType>
	inline decltype(auto) endTime(const std::chrono::time_point<std::chrono::steady_clock>& startTime)
	{
		return duration_cast<TimeType>(high_resolution_clock::now() - startTime).count();
	}
}
