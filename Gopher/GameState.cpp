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
	int stateIdx	= 0;
	int slIdx		= BoardSize2;

	for (int i = 0; (i < moveCount && i < BoardHistory); ++i)
	{
		++stateIdx;
		for (int c = 0; c < 2; ++c)
		{
			int idx = 0;
			for (int y = 0; y < BoardSize; ++y)
				for (int x = 0; x < BoardSize; ++x)
				{
					// Index into the padded state vector that matches
					// our idx
					const int pIdx = getPaddedIdx(x, y);

					// Index into the the particular slice in the 1D slices vector
					//const int inIdx = stateIdx * (c+1) * BoardSize2 + idx;

					// TODO: VERIFY THIS IS WORKING AS INTENDED
					// 
					// TODO: Pass trivial(simple/repeatable) but non zero input into the net
					// here and in the python CNTK trainer and verify output is identical (also verify
					// that the orientation in the 1D array is correct here)
					input.slices[slIdx] = static_cast<float>(states[moveCount - stateIdx][pIdx] == c+1);
					++slIdx;
					++idx;
				}
		}
	}
	
	// Debug
	// TODO: Add a function to verify slices and to a
	// limited extent states against the current board!
	//
	//printStates();
	//printSlices(input);

	return input;
}

template<class It>
void printBoardVec(It begin, It end, int size)
{
	std::cerr << '\n';
	int i = 0;
	for (auto it = begin; it != end; ++it)
	{
		std::cerr << *it << ' ';
		++i;
		if (i >= size)
		{
			std::cerr << '\n';
			i = 0;
		}
	}
	std::cerr << '\n';
}

void GameState::printSlices(const NetInput & input) const
{
	int idx = 0;
	for (int i = 0; i < moveCount * 2; ++i)
	{
		auto begin	= input.slices.begin() + idx;
		auto end	= begin + BoardSize2;
		printBoardVec(begin, end, BoardSize);
		idx += BoardSize2;
	}
}

void GameState::printStates() const
{
	for (int i = 0; i < moveCount; ++i)
	{
		auto begin	= states[i].begin();
		auto end	= states[i].end();
		printBoardVec(begin, end, BoardRealSize);
	}
}
