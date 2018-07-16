#include "UctNode.h"
#include "Net.h"
#include "Board.h"

inline int idxToRealIdx(int idx)
{
	const int x = idx % BoardSize;
	const int y = idx / BoardSize;
	return (y + 1) * BoardRealSize + (x + 1);
}

void UctNode::expand(const GameState& state, const Board& board, const NetResult& result, int color)
{
	expanded = true;

	// Store win chance as black
	setEval(result, color);

	int idx = -1;
	for (const auto moveProb : result.moves())
	{
		++idx;
		const int16_t rIdx = idxToRealIdx(idx);
		if (!board.isValidNoSuicide({ rIdx, color }))
			continue;

		// TODO: Think about how we should bonus the nodes scored high here
		// TODO: Optimize memory allocations ~ They're very poorly done here
		children.emplace_back(UctNode{ rIdx, moveProb });
	}
}

UctNode* UctNode::selectChild(int color, bool isRoot)
{
	int idx		= 0;
	int bestIdx = 0;
	static constexpr double UCT_EXPLORE = 0.35;
	//static constexpr double FPU_REDUCTION = 0.25;
	double best = std::numeric_limits<double>::lowest();

	// TODO: Look into reducing the estimated eval of 
	// low quality moves
	//
	auto numerator	= std::sqrt(static_cast<double>(this->visits));
	auto fpuEval	= getEval(color); // Set estimated eval equal to parent eval

	for (const auto& child : children)
	{
		auto winrate = fpuEval;
		if (child.visits)
			winrate = (static_cast<double>(child.wins)
					/  static_cast<double>(child.visits));

		auto psa	= child.prior;
		auto denom	= 1.0 + child.visits;
		auto uct	= UCT_EXPLORE * psa * (numerator / denom);
		auto val	= winrate + uct;

		if (val > best)
		{
			bestIdx = idx;
			best = val;
		}
		++idx;
	}

	return &children[bestIdx];
}

bool UctNode::isExpanded()
{
	return expanded;
}

void UctNode::scoreNode(bool win)
{
	//wins += color == BLACK ? (result <= 0) : (result >= 0);
	wins += win;
}

void UctNode::setEval(const NetResult & result, int color)
{
	value = result.winChance(NetResult::TO_MOVE);
	if (color == WHITE)
		value = 1.0 - value;
}

float UctNode::getEval(int color) const
{
	if (color == WHITE)
		return 1.0 - value;
	return value;
}

bool UctNode::isWin(int color) const
{
	return getEval(color) > 0.5;
}

