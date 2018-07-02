#pragma once
#include "Board.h"
#include <vector>

// How many states do we hold on to?
static constexpr int BoardHistory = 3;

class GameState
{
public:
	using BoardState = std::vector<int>;

	GameState() = default;

	int moves = 0;

	// Where are we going to add the latest state to
	int stateIdx = 0;
	std::vector<BoardState> states;

	void makeMove(const Board& board);
	void popState();
	void clear();
};

