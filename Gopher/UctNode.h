#pragma once
#include <vector>

class GameState;
struct NetResult;
struct Board;
using coord = int;

// TODO: Non expanded nodes on init like leelaZero
struct UctNode
{
	UctNode() = default;
	UctNode(int16_t idx, float prior);

	using Children = std::vector<UctNode>;
									// TODO: Should these default inits be removed? 
									// Probably cost something heavy with all the vector constructing?
	int16_t		idx;				// Index of move on padded board
	float		policy;				// Policy networks output, how much expert likes move
	float		value		= 0.f;	// Evaluation network gave board state
	int			visits		= 0;	// Total visits passing through node
	float		wins		= 0.0;	// Wins from blacks perspective
	bool		expanded	= false;	
	Children*	children	= nullptr;

	void		del();
	void		expand(const GameState& state, const Board& board, const NetResult& result, int color);
	UctNode&	selectChild(int color, bool isRoot) const;
	UctNode*	findChild(int idx) const;
	bool		isExpanded() const;
	bool		empty() const;
	void		update(float eval);

	// Set the network evaluation from blacks perspective
	void		setNetEval(const NetResult& result, int color);
	// Get the origional Value networks 
	// value of this node accorrding to color
	float		getNetEval(int color) const;
	// Get the current evaluation of this node
	// (wins / visits) according to color
	float		getEval(int color) const;

};

struct UctNodePred
{
	UctNodePred(int color) :color(color) {}
	bool operator()(const UctNode& n0, const UctNode& n1)
	{
		if (n0.visits != n1.visits)
			return n0.visits < n1.visits;
		if (n0.visits == 0)
			return n0.policy < n1.policy;

		return n0.getEval(color) < n1.getEval(color);
	}
	int color;
};
