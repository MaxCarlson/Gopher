#include "Tree.h"
#include "UctNode.h"
#include "Board.h"
#include "GameState.h"
#include "Net.h"
#include <algorithm>
#include <memory>

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

	if (!getRoot().isExpanded())
	{
		NetResult netResult = Net::run(state, color);
		getRoot().expand(state, board, netResult, color);
	}
	else
		root->visits = root->wins = 0;
}

UctNode& findBestMove()
{
	// TODO: Should we look at both wins and win ratio?
	auto bestNode  = std::max_element(
		std::begin(*root->children), 
		std::end(*root->children), 
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
	auto* newRoot = new UctNode{ best };
	moveNewRoot(best, newRoot);
	root->del();
	root.reset(newRoot);
}

} // End Tree::


