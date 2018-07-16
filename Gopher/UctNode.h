#pragma once
#include <vector>

class GameState;
struct NetResult;
struct Board;
using coord = int;

struct UctNode
{
	UctNode() = default;
	UctNode(int16_t idx, float prior)
		: idx(idx), prior(prior) {}

	int16_t		idx;
	float		prior;
	float		value = 0.f;
	int			visits = 0;
	int			wins = 0;
	bool		expanded = false;


	void expand(const GameState& state, const Board& board, const NetResult& result, int color);
	UctNode* selectChild(int color, bool isRoot);
	bool isExpanded();
	void scoreNode(bool win);

	void setEval(const NetResult& result, int color);
	float getEval(int color) const;
	bool isWin(int color) const;

	std::vector<UctNode> children;
};