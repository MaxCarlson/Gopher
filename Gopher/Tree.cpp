#include "Tree.h"
#include "Net.h"
#include "Board.h"
#include "Random.h"

inline int idxToRealIdx(int idx)
{
	const int x = idx % BoardSize;
	const int y = idx / BoardSize;
	return (y + 1) * BoardRealSize + (x + 1);
}

void UctNode::expand(const GameState& state, const Board& board, const NetResult& result, int color)
{
	expanded = true;
	winVal = result.winChance(NetResult::TO_MOVE);

	int idx = 0;
	for (const auto score : result.moves())
	{
		auto rIdx = idxToRealIdx(idx);
		if (!board.isValidNoSuicide({ rIdx, color }))
		{
			++idx;
			continue;
		}

		// TODO: Think about how we should bonus the nodes scored high here
		// TODO: Optimize memory allocations ~ They're very poorly done here
		auto node = UctNode{};
		node.wins += score;
		node.idx = rIdx;
		children.emplace_back(node);
		++idx;
	}
}

UctNode* UctNode::selectChild(int color)
{
	int idx = 0;
	int bestIdx = 0;
	static constexpr double UCT_EXPLORE = 0.35;
	double best = std::numeric_limits<double>::lowest();

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

namespace Tree
{


coord findBestMove()
{
	// TODO: Should we look at both wins and win ratio?
	auto bestNode  = std::max_element(
		std::begin(root.children), 
		std::end(root.children), 
		[&](const UctNode& node0, const UctNode& node1)
	{
		return node0.wins < node1.wins;
	});

	return bestNode->idx;
}


}


