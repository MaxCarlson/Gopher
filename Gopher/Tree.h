#pragma once
#include <vector>

class GameState;
struct Board;

struct UctNode
{
	int wins		= 0;
	int visits		= 0;
	bool expanded	= false;

	void expand(const GameState& state, const Board& board, int color);
	void selectChild(int color);
	bool isExpanded();

	std::vector<UctNode> children;
};

