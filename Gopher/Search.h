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
	UctNode root;

	void playout(Board board, GameState& state, UctNode* const node, int color);
	void walkTree(Board& board, GameState& state, int& color);
};

