#include "GameState.h"

void GameState::makeMove(const Board & board)
{
	if (moveCount >= states.size())
	{
		states.emplace_back(BoardState{});
		states[moveCount].resize(BoardRealSize2);
	}

	// Copy the board's points into the next state
	std::copy_n(board.points, BoardRealSize2, states[moveCount++].data());
}

void GameState::popState()
{
	// Don't remove states that might later be filled again
	--moveCount;
}

void GameState::clear()
{
	moveCount = 0;
}

inline int getPaddedIdx(int x, int y)
{
	return (y + 1) * BoardRealSize + (x + 1);
}

NetInput GameState::genNetInput(int color) const
{
	NetInput input;
	input.slices.resize(InputSize, 0.f);

	// Fill the color slice with appropriate color
	// TODO: Should we branch here or no?
	std::fill_n(input.slices.data(), BoardSize2, static_cast<float>(color - 1));
	
	// Fill the rest of the binary slices
	int sliceIdx = 0;
	for (int i = 0; (i < moveCount && i < BoardHistory); i += 2)
	{
		++sliceIdx;
		for (int c = 0; c < 2; ++c)
		{
			int idx = 0;
			for (int y = 0; y < BoardSize; ++y)
				for (int x = 0; x < BoardSize; ++x)
				{
					const int pIdx = getPaddedIdx(x, y);
					const int inIdx = sliceIdx * BoardSize2 + idx;

					// TODO: VERIFY THIS IS WORKING AS INTENDED
					// 
					// TODO: Pass trivial(simple/repeatable) but non zero input into the net
					// here and in the python CNTK trainer and verify output is identical (also verify
					// that the orientation in the 1D array is correct here)
					input.slices[inIdx] = static_cast<float>(states[moveCount - sliceIdx][pIdx] == c);
					++idx;
				}
		}
	}
	

	return input;
}
