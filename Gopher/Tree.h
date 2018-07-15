#pragma once
#include <vector>

class GameState;
class NetResult;
struct Board;
using coord = int;

struct UctNode
{
	int16_t idx		= 0;
	int wins		= 0;
	int visits		= 0;
	float winVal	= 0.f;
	bool expanded	= false;

	void expand(const GameState& state, const Board& board, const NetResult& result, int color);
	UctNode* selectChild(int color);
	bool isExpanded();
	void scoreNode(bool win);

	std::vector<UctNode> children;
};

namespace Tree
{
	inline UctNode root;

	coord findBestMove();
}

