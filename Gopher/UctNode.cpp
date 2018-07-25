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
	children = new Children();
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
	expanded = false;
}

void UctNode::expand(const GameState& state, const Board& board, const NetResult& result, int color)
{
	expanded = true;

	// Store win chance as black 
	// (Net returns win chance as side to move)
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
	auto idx     = 0;
	auto bestIdx = 0;
	static constexpr double UCT_EXPLORE = 0.75; // Doesn't seem to effect search much at all!
	auto best	 = std::numeric_limits<float>::lowest();

	// TODO: Look into reducing the estimated eval of 
	// low quality moves
	//
	// TODO: ? Numerator as it is makes all child policy values worthless
	// if parent visits is zero. Test whether or not we should add one to the numerator!
	// Probably in combination with not expanding leaf nodes until n visits
	const auto parentVis	= std::sqrt(static_cast<float>(this->visits));
	const auto pNetEval		= getNetEval(color); // Set estimated eval equal to parent eval

	for (const auto& child : *children)
	{
		// Use parent eval if child's hasn't been
		// evaluated by network
		auto winrate = pNetEval;
		if (child.visits)
			winrate = child.getEval(color);

		auto psa		= child.policy;
		auto childVis	= 1.f + child.visits;
		auto uct		= UCT_EXPLORE * psa * (parentVis / childVis); // UCT_EXPLORE * Appears to do nothing much
		auto val		= winrate + uct;

		if (val > best)
		{
			bestIdx	= idx;
			best	= val;
		}
		++idx;
	}

	// TODO: Unlikey or never? Child chosen with -policy
	//auto& child = children->at(bestIdx);
	//if (child.policy <= 0.f)
	//	int a = 5;

	return (*children)[bestIdx];
}

UctNode* UctNode::findChild(int idx) const
{
	if (!children)
		return nullptr;

	auto childIt = std::find_if(
		std::begin(*children), 
		std::end(*children), 
		[&](UctNode& n)
	{
		return n.idx == idx; 
	});
	
	UctNode* child = nullptr;
	if (childIt != std::end(*children))
		child = &*childIt;
	return child;
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
	// Record netEval from blacks perspective
	value = result.winChance(NetResult::TO_MOVE);
	if (color == WHITE)
		value = 1.f - value;
}

float UctNode::getNetEval(int color) const
{
	if (color == WHITE)
		return 1.f - value;
	return value;
}

float UctNode::getEval(int color) const
{
	const auto score = wins / static_cast<float>(visits);
	if (color == WHITE)
		return 1.f - score;
	return score;
}
