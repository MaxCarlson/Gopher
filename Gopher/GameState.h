#pragma once
#include "Board.h"
#include <array>
#include <vector>

// Holds all the previous moves made 
// in a vector stack. Used for feeding NetInput
//
// 
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
