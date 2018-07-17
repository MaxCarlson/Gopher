#pragma once
#include <vector>

class GameState;
struct NetResult;
struct Board;
using coord = int;

struct UctNode
{
	UctNode() = default;
	UctNode(int16_t idx, float prior);		

	int16_t		idx;				// Index of move on padded board
	float		policy;				// Policy networks output, how much expert likes move
	float		value		= 0.f;	// Evaluation network gave board state
	int			visits		= 0;	// Total visits passing through node
	float		wins		= 0.0;	// Wins from blacks perspective
	bool		expanded	= false;

	void del();
	void expand(const GameState& state, const Board& board, const NetResult& result, int color);
	UctNode* selectChild(int color, bool isRoot);
	bool isExpanded() const;
	bool empty() const;
	void update(float eval);

	// Set the network evaluation from blacks perspective
	void setNetEval(const NetResult& result, int color);
	// Get the origional Value networks 
	// value of this node accorrding to color
	float getNetEval(int color) const;
	// Get the current evaluation of this node
	// (wins / visits) according to color
	float getEval(int color) const;
	bool isWin(int color) const;

	std::vector<UctNode>* children = nullptr;
};