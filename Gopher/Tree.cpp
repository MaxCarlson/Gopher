#include "Tree.h"
#include "Net.h"
#include "Board.h"
#include "Random.h"

inline int idxToRealIdx(int idx)
{
	const int x = idx % BoardSize;
	const int y = idx / BoardSize;
	return (y + 1) * BoardRealSize2 + (x + 1);
}

void UctNode::expand(const GameState& state, const Board& board, int color)
{
	expanded = true;
	NetResult result = Net::run(state, color);

	int idx = 0;
	for (const auto score : result.moves())
	{
		auto rIdx = idxToRealIdx(idx);
		if (!board.isValidNoSuicide({ rIdx, color }))
			continue;

		// TODO: Think about how we should bonus the nodes scored high here
		// TODO: Optimize memory allocations ~ They're very poorly done here
		auto node = UctNode{};
		node.wins += score;
		children.emplace_back(node);
		++idx;
	}
}

void UctNode::selectChild(int color)
{
	int idx = 0;
	int bestIdx = 0;
	static constexpr double UCT_EXPLORE = 0.35;
	double best = std::numeric_limits<double>::min();

	for (const auto& child : children)
	{
		double val; 

		if (child.visits == 0)
			val = 10000.0 + static_cast<double>(Random::fastRandom(10000));
		else
			val = (static_cast<double>(child.wins)
				/ static_cast<double>(child.visits))
			    + UCT_EXPLORE * std::sqrt(std::log(this->visits) / child.visits);

		if (val > best)
		{
			bestIdx = idx;
			best = val;
		}
		++idx;
	}

}

bool UctNode::isExpanded()
{
	return expanded;
}
