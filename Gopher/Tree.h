#pragma once
#include <vector>

class GameState;
class NetResult;
struct Board;
using coord = int;

struct UctNode
{
	UctNode() = default;
	UctNode(int16_t idx, float prior) 
		: idx(idx), prior(prior) {}

	int16_t		idx;
	float		prior;
	float		value		= 0.f;
	uint16_t	visits		= 0;
	int16_t		wins		= 0;
	bool		expanded	= false;


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

