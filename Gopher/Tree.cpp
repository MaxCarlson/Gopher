#include "Tree.h"
#include "UctNode.h"
#include <algorithm>

namespace Tree
{

UctNode root;

UctNode & getRoot()
{
	return root;
}

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

} // End Tree::


