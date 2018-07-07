#include "GameState.h"

void GameState::makeMove(const Board & board)
{
	if (stateIdx >= states.size())
	{
		states.emplace_back(BoardState{});
		states[stateIdx].resize(BoardRealSize2);
	}

	// Copy the board's points into the next state
	std::copy_n(board.points, BoardRealSize2, states[stateIdx++].data());
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
	input.slices.resize(InputSize, 0.f);

	// Fill the color slice with appropriate color
	if(color == WHITE)
		std::fill_n(input.slices.data(), BoardSize2, static_cast<float>(color - 1));
	
	// Fill the rest of the binary slices
	int sliceIdx = 0;
	for (int i = 0; (i < moves && i < BoardHistory); i += 2)
	{
		++sliceIdx;
		for (int c = 0; c < 2; ++c)
		{
			int idx = 0;
			for (int x = 0; x < BoardSize; ++x)
				for (int y = 0; y < BoardSize; ++y)
				{
					const int pIdx = getPaddedIdx(x, y);
					const int inIdx = sliceIdx * BoardSize2 + idx;

					// TODO: VERIFY THIS IS WORKING AS INTENDED
					// 
					// TODO: Pass trivial(simple/repeatable) but no zero input into the net
					// here and in the python CNTK trainer and verify output is identical (also verify
					// that the orientation in the 1D array is correct here)
					input.slices[inIdx] = static_cast<float>(states[stateIdx - sliceIdx][pIdx] == c);
					++idx;
				}
		}
	}
	

	return input;
}
