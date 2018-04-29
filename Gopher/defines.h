#pragma once

constexpr int BoardSize = 19;
constexpr int BoardRealSize = BoardSize + 2;
constexpr int BoardRealSize2 = BoardRealSize * BoardRealSize;
constexpr int BoardOffset = 2;
constexpr int BoardMaxIdx = (BoardSize + BoardOffset) * (BoardSize + BoardOffset);
constexpr int BoardMaxGroups = BoardSize * BoardSize * 2 / 3;

