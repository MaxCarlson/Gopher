#include "Tree.h"
#include "UctNode.h"
#include "Board.h"
#include "GameState.h"
#include "Net.h"
#include "Stone.h"
#include <algorithm>
#include <memory>
#include <map>
#include <iomanip>

namespace Tree
{

std::unique_ptr<UctNode> root;

UctNode & getRoot()
{
	return *root.get();
}

void initRoot(const Board & board, GameState & state, int color)
{
	if (!root)
		root = std::make_unique<UctNode>(Pass, 0.f);

	if (!root->isExpanded())
	{
		NetResult netResult = Net::run(state, color);
		root->expand(state, board, netResult, color);
		root->update(root->getNetEval(BLACK));
	}
	
	auto eval = root->getEval(color);
	std::cerr << "Net eval for " << stoneString(color) << " " << eval << '\n';
}

UctNode& findBestMove(UctNode* node)
{
	// TODO: Should we look at both wins and win ratio?
	auto bestNode  = std::max_element(
		std::begin(*node->children), 
		std::end(*node->children), 
		[&](const UctNode& node0, const UctNode& node1)
	{
		return node0.visits < node1.visits;
	});

	return *bestNode;
}

// Make sure the newRoots children aren't deleted with
// the rest of the obsolete tree
void moveNewRoot(UctNode& best, UctNode* newRoot)
{
	for(auto& child : *root->children)
		if (best.idx == child.idx)
		{
			child.children = nullptr;
			break;
		}
}

void switchRoot(UctNode& best)
{
	// TODO: Do this on another thread!
	// TODO: Add thread pool to do it in!
	auto* newRoot = new UctNode{ best };
	moveNewRoot(best, newRoot);
	root->del();
	root.reset(newRoot);
}

void printStats(int color)
{
	auto* node = &findBestMove(root.get());

	int ccolor = color;

	std::cerr << "Line: ";

	// This could cause issues with empty/malformed nodes?
	while (!node->empty())
	{
		std::cerr << moveToString(node->idx) << " " << node->getEval(ccolor) << ", ";
		ccolor = flipColor(color);

		node = &findBestMove(node);
	}
	std::cerr << '\n';
}

void printNodeInfo(UctNode * node)
{
	// Print histogram of visit counts
	// and their average network expert score
	std::map<int, std::pair<int, float>> hist;
	if (node->empty())
		return;

	for (const auto& child : *node->children)
	{
		auto find = hist.find(child.visits);
		if (find == hist.end())
			hist.emplace(child.visits, std::make_pair(1, child.policy));
		else
		{
			++find->second.first;
			find->second.second += child.policy;
		}
	}
	std::cerr << "Visits, Count, Avg Policy \n";
	for (const auto& h : hist)
		std::cerr << std::setw(3) << h.first
		<< ", " << std::setw(3) << h.second.first << ", "
		<< h.second.second / h.second.first << '\n';
}

} // End Tree::


