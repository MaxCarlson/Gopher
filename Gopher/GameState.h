#pragma once
#include "Board.h"
#include <array>
#include <vector>

// How many states do we hold on to?
static constexpr int BoardHistory = 3;
// Move these to defines once relevent for storage
static constexpr int LayersPerState = 2;
static constexpr int BoardDepth = LayersPerState * BoardHistory + 1;
static constexpr int InputSize = BoardDepth * BoardSize2;

using NetSlice = std::vector<float>;

class GameState
{
public:
	// Should these be stored as floats for the network?
	using BoardState = std::vector<int>;

	GameState() = default;

	// Where are we going to add the latest state to
	int moveCount = 0;
	std::vector<BoardState> states;

	void makeMove(const Board& board);
	void popState();
	void clear();

	void printStates() const;
};

struct NetInput
{
	NetInput(const GameState& state, int color);
	void printSlices(const GameState& state) const;

	std::vector<float> slices;
};
