#include "SearchTree.h"
#include "Board.h"


size_t UctNodeBase::size() const
{
	return children ? children->nodes.size() : 0;
}

bool UctNodeBase::isLeaf() const
{
	return !children;
}

void SearchTree::init(const Board& board, int color) 
{
	baseColor = color;
	expandNode(board, root, color);
	++root.visits;
}

void SearchTree::allocateChildren(UctNodeBase & node)
{
	node.children = new UctTreeNodes;
}

void SearchTree::expandNode(const Board & board, UctNodeBase & node, int color)
{
	if (!board.free.size())
		return;

	if (!node.children)
		allocateChildren(node);


	//int children = 0;
	board.foreachFreePoint([&](coord idx)
	{
		if (!board.isValidNoSuicide({ idx, color }))
			return;

		node.children->nodes.emplace_back(idx); // Likely optimization point
	});
}

void SearchTree::recordSearchResults(SmallVec<int, 100>& moves, int color, bool isWin)
{
	if (!isWin)
		return;

	UctNodeBase* node = &root;
	node->wins += isWin;

	// Loop through all nodes we moved through 
	// and score them relative to side to move
	int depth = 0;
	for (const auto it : moves)
	{
		node = &node->children->nodes[it];
		node->wins += isWin;

		std::cout << "NodeInfo: Depth-" << depth++ << " Visits-" << node->visits << " Wins-" << node->wins << '\n';
	}
	std::cout << '\n';
}

