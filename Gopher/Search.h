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

	float playout(Board& board, GameState& state, UctNode& node, int depth, int color);
};

inline Search search;

