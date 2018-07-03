#include "GameState.h"

void GameState::makeMove(const Board & board)
{
	if (stateIdx >= states.size())
	{
		states.emplace_back(BoardState{});
		states[stateIdx].resize(BoardRealSize2);
	}

	// Copy the board's points into the next state
	std::copy_n(board.points, BoardRealSize2, states[stateIdx++]);
}

void GameState::popState()
{
	--stateIdx;
	// Don't remove states that might later be filled again
}

void GameState::clear()
{
	stateIdx = 0;
}

inline int getPaddedIdx(int x, int y)
{
	return (y + 1) * BoardRealSize + (x + 1);
}

NetInput GameState::genNetInput(int color) const
{
	NetInput input;

	// Fill the color slice with appropriate color
	std::fill_n(input.slices[0], BoardSize2, static_cast<float>(color - 1));
	
	// Fill the rest of the binary slices
	int sliceIdx = 1;
	for (int i = 0; (i < moves && i < BoardHistory); ++i)
	{
		for (int c = 0; c < 2; ++c)
		{
			++sliceIdx;
			int idx = 0;
			for (int x = 0; x < BoardSize; ++x)
				for (int y = 0; y < BoardSize; ++y)
				{
					const int pIdx = getPaddedIdx(x, y);
					input.slices[sliceIdx][idx] = static_cast<float>(states[stateIdx - sliceIdx][pIdx]);
					++idx;
				}
		}
	}

	return input;
}
