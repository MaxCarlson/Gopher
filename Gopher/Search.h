#pragma once
#include "Tree.h"

struct Board;
class GameState;
using coord = int;


class Search
{
public:
	coord search(const Board& board, GameState& state, int color);

private:

	float playout(Board& board, GameState& state, UctNode& node, int& depth, int color);
	bool resignOrPass(const GameState& state, const UctNode* best, coord& idx, int color) const;
	void moveNoise(UctNode*& best, int color) const;
};

inline Search search;

