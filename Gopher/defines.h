#pragma once

constexpr int BoardSize = 19;
constexpr int BoardRealSize = BoardSize + 2;
constexpr int BoardRealSize2 = BoardRealSize * BoardRealSize;
constexpr int BoardOffset = 2;
constexpr int BoardMaxIdx = (BoardSize + BoardOffset) * (BoardSize + BoardOffset);
constexpr int BoardMaxGroups = BoardSize * BoardSize * 2 / 3;

#include <vector>

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

	static const auto flipper = createFlipper();;

	return flipper[y];
}