#include "UctNode.h"
#include "Net.h"
#include "Board.h"

inline int idxToRealIdx(int idx)
{
	const int x = idx % BoardSize;
	const int y = idx / BoardSize;
	return (y + 1) * BoardRealSize + (x + 1);
}

UctNode::UctNode(int16_t idx, float prior) : idx(idx), policy(prior) 
{
	children = new std::vector<UctNode>();
}

void UctNode::del()
{
	if (children)
	{
		for (auto& child : *children)
			child.del();
		delete children;
		children = nullptr;
	}
		
}

void UctNode::expand(const GameState& state, const Board& board, const NetResult& result, int color)
{
	expanded = true;

	// Store win chance as black
	setNetEval(result, color);

	int idx = -1;
	for (const auto moveProb : result.moves())
	{
		++idx;
		const int16_t rIdx = idxToRealIdx(idx);
		if (!board.isValidNoSuicide({ rIdx, color }))
			continue;

		// TODO: Think about how we should bonus the nodes scored high here
		// TODO: Optimize memory allocations ~ They're very poorly done here
		children->emplace_back(UctNode{ rIdx, moveProb });
	}
}

UctNode& UctNode::selectChild(int color, bool isRoot) const
{
	int idx		= 0;
	int bestIdx = 0;
	static constexpr double UCT_EXPLORE = 0.95; // Doesn't seem to effect search much at all!
	//static constexpr double FPU_REDUCTION = 0.25;
	double best = std::numeric_limits<double>::lowest();

	// TODO: Look into reducing the estimated eval of 
	// low quality moves
	//
	// TODO: ? Numerator as it is makes all child policy values worthless
	// if parent visits is zero. Test whether or not we should add one to the numerator!
	// Probably in combination with not expanding leaf nodes until n visits
	auto parentVis	= std::sqrt(static_cast<double>(this->visits));
	auto fpuEval	= getNetEval(color); // Set estimated eval equal to parent eval

	for (const auto& child : *children)
	{
		// Use parent eval if child's hasn't been
		// evaluated by network
		auto winrate = fpuEval;
		if (child.visits)
			winrate = child.getEval(color);

		auto psa		= child.policy;
		auto childVis	= 1.0 + child.visits;
		auto uct		= UCT_EXPLORE * psa * (parentVis / childVis);
		auto val		= winrate + uct;

		if (val > best)
		{
			bestIdx = idx;
			best = val;
		}
		++idx;
	}

	return (*children)[bestIdx];
}

bool UctNode::isExpanded() const
{
	return expanded;
}

bool UctNode::empty() const
{
	if (!children)
		return true;
	return children->empty();
}

void UctNode::update(float eval)
{
	++visits;
	wins += eval;
}

void UctNode::setNetEval(const NetResult & result, int color)
{
	value = result.winChance(NetResult::TO_MOVE);
	if (color == WHITE)
		value = 1.0 - value;
}

float UctNode::getNetEval(int color) const
{
	if (color == WHITE)
		return 1.0 - value;
	return value;
}

float UctNode::getEval(int color) const
{
	const auto score = wins / static_cast<float>(visits);
	if (color == WHITE)
		return 1.0 - score;
	return score;
}

bool UctNode::isWin(int color) const
{
	return getEval(color) > 0.5;
}
