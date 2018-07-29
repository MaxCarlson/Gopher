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

void verifyRoot(const Board & board, GameState & state, int color)
{
	if (!root)
		root = std::make_unique<UctNode>(Pass, 0.f);

	if (!root->isExpanded())
	{
		NetResult netResult = Net::inference(state, color);
		root->expand(state, board, netResult, color);
		root->update(root->getNetEval(BLACK));
	}
}

void initRoot(const Board & board, GameState & state, int color, bool reset)
{	
	if (reset)
		root->del();

	verifyRoot(board, state, color);

	auto eval = root->getEval(color);
	std::cerr << "Net eval for " << stoneString(color) << " root " << eval << '\n';
}

UctNode* findBestMove(UctNode* node, int color)
{
	if (node->empty())
		return nullptr;

	// TODO: Should we look at both wins and win ratio?
	auto bestNode  = std::max_element(
		std::begin(*node->children), 
		std::end(*node->children), 
		UctNodePred{ color }
	);

	return &*bestNode;
}

void updateRoot(const Board & board, GameState& state, int color, int bestIdx)
{
	if (!root || !root->isExpanded())
		verifyRoot(board, state, color);

	auto* child = root->findChild(bestIdx);

	updateRoot(*child);
}

void updateRoot(UctNode& best)
{
	// TODO: Do this on another thread!
	// TODO: Add thread pool to do it in!
	auto* newRoot = new UctNode{ best };

	// Make sure we don't delete newRoots children 
	// with the rest of the obsolete tree
	auto* child = root->findChild(best.idx);
	if (child)
		child->children = nullptr;

	root->del();
	root.reset(newRoot);
}

void printStats(int color)
{
	int ccolor = color;
	auto* node = findBestMove(root.get(), color);

	// This could cause issues with empty/malformed nodes?
	std::cerr << "Line: ";
	while (!node->empty())
	{
		std::cerr << moveToString(node->idx) << " "
			<< std::fixed << std::setprecision(2) << node->getEval(ccolor) << ", ";
		ccolor = flipColor(ccolor);

		node = findBestMove(node, ccolor);
	}
	std::cerr << '\n';
}

// Print histogram of visit counts
// and their average network expert score
void printNodeInfo(UctNode * node, int color)
{
	std::map<int, std::pair<int, std::vector<const UctNode*>>> hist;
	if (node->empty())
		return;

	for (const auto& child : *node->children)
	{
		auto find = hist.find(child.visits);
		if (find == hist.end())
			hist.emplace(child.visits, std::make_pair(1, std::vector<const UctNode*>{ &child }));
		else
		{
			++find->second.first;
			find->second.second.emplace_back(&child);
		}
	}

	std::cerr << "Visits, Count, AvgPolicy, AvgEval, Coords, idx, NetValue \n";
	for (const auto& h : hist)
	{
		auto avgPol		= 0.0;
		auto avgEval	= 0.0;
		for (const auto& c : h.second.second)
		{
			avgPol += c->policy;
			avgEval = c->getEval(color);
		}
		avgPol	/= h.second.second.size();
		avgEval /= h.second.second.size();

		std::cerr << std::setw(6) << h.first
			<< ", " << std::setw(5) << h.second.first << ", " << std::setw(9)
			<< avgPol << ", " << std::setw(7) << std::right << avgEval << ',';

		const auto& node = h.second.second[0];
		if (h.second.second.size() == 1)
			std::cerr << std::setw(7) << std::right << moveToString(node->idx)
			<< ", " << std::setw(3) << std::right << node->idx << ", " << std::setw(8) << node->value;
		std::cerr << '\n';
	}
		
}

} // End Tree::


