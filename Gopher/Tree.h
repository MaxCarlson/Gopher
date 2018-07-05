#pragma once
#include <vector>

class GameState;
struct Board;

struct UctNode
{
	int16_t idx		= 0;
	int wins		= 0;
	int visits		= 0;
	bool expanded	= false;

	void expand(const GameState& state, const Board& board, int color);
	UctNode* selectChild(int color);
	bool isExpanded();
	void scoreNode(int result, int color);

	std::vector<UctNode> children;
};

