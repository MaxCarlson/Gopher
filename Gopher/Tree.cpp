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
	value    = result.winChance(NetResult::TO_MOVE);

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

UctNode* UctNode::selectChild(int color)
{
	int idx = 0;
	int bestIdx = 0;
	static constexpr double UCT_EXPLORE = 0.35;
	double best = std::numeric_limits<double>::lowest();

	for (const auto& child : children)
	{
		double val; 

		/* // Formal UCT
		if (child.visits == 0)
			val = child.prior; //10000.0 + static_cast<double>(Random::fastRandom(10000));
		else
			val = (static_cast<double>(child.wins)
				/ static_cast<double>(child.visits))
			    + UCT_EXPLORE * std::sqrt(std::log(this->visits) / child.visits);
		*/

		auto usa = child.prior / static_cast<double>(1 + child.visits);
		auto uct = (static_cast<double>(child.wins)
				 / static_cast<double>(child.visits))
				 + UCT_EXPLORE * std::sqrt(std::log(this->visits) / child.visits);

		val = uct + usa;

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
		return node0.visits < node1.visits;
	});

	return bestNode->idx;
}


}


