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

NetInput GameState::genNetInput(int color) const
{
	NetInput input;

	std::fill_n(input.slices[0], BoardSize2, static_cast<float>(color - 1));
	
	for (int i = 0; (i < moves && i < BoardHistory); ++i)
	{

	}
}
