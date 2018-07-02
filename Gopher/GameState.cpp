#include "GameState.h"

void GameState::makeMove(const Board & board)
{
	if (stateIdx >= states.size())
		states.emplace_back(BoardState{});

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
