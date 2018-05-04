#include "SearchTree.h"
#include "Board.h"


size_t UctNodeBase::size() const
{
	return children ? children->nodes.size() : 0;
}

void SearchTree::init(const Board& board, int color) 
{
	baseColor = color;
	expandNode(board, root, color);
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

