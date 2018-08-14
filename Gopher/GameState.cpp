#include "GameState.h"

void GameState::makeMove(const Board & board)
{
	if (moveCount >= states.size())
	{
		states.emplace_back(BoardState{});
		states[moveCount].resize(BoardRealSize2);
	}
	// Copy the board's points into the next state
	//std::copy_n(board.points, BoardRealSize2, states[moveCount++].data());
	std::copy(std::begin(board.points), std::end(board.points), std::begin(states[moveCount++]));
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

template<class It>
void printBoardVec(It begin, It end, int size)
{
	std::cerr << '\n';
	int i = 0;
	for (auto it = begin; it != end; ++it)
	{
		std::cerr << *it << ' ';
		// Format the new lines based on 
		// size of board length
		++i;
		if (i >= size)
		{
			std::cerr << '\n';
			i = 0;
		}
	}
	std::cerr << '\n';
}

void GameState::printStates() const
{
	for (int i = moveCount - 1; i >= 0; --i)
	{
		auto begin	= states[i].begin();
		auto end	= states[i].end();
		printBoardVec(begin, end, BoardRealSize);
	}
}

NetInput::NetInput(const GameState& state, int color)
{
	slices.resize(InputSize, 0.f);

	// Fill the color slice with appropriate color
	// TODO: Should we branch here or no?
	std::fill_n(slices.data(), BoardSize2, static_cast<float>(color - 1));

	// Fill the rest of the binary slices
	int stateOffset = 0;
	int slIdx = BoardSize2;
	for (int i = 0; (i < state.moveCount && i < BoardHistory); ++i)
	{
		int stateIdx = state.moveCount - ++stateOffset;
		for (int c = color; c != OFFBOARD; )
		{
			for (int y = 0; y < BoardSize; ++y)
				for (int x = 0; x < BoardSize; ++x)
				{
					// Index into the padded state vector that matches
					// our idx
					int pIdx = getPaddedIdx(x, y);

					// TODO: VERIFY THIS IS WORKING AS INTENDED
					// 
					// TODO: Pass trivial but non zero input into the net
					// here and in the python CNTK trainer and verify output is identical (also verify
					// that the orientation in the 1D array is correct here)
					slices[slIdx] = static_cast<float>(state.states[stateIdx][pIdx] == c);
					++slIdx;
				}
			c = c == color ? flipColor(c) : OFFBOARD;
		}
	}
	// TODO: Some really weird issues with cerr here sometimes
	// Occasionally it will print out JUNK that isn't in the array,
	// the second print statement won't print it
	//
	//printSlices(state);
	//printSlices(state);
}

void NetInput::printSlices(const GameState& state) const
{
	for (int sidx = 0; sidx < slices.size(); sidx += BoardSize2)
	{
		auto begin	= slices.begin() + sidx;
		auto end	= begin + BoardSize2;

		printBoardVec(begin, end, BoardSize);
	}
}