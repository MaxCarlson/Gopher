#include "SearchTree.h"
#include "Board.h"





void SearchTree::allocateChildren(UctNodeBase & node)
{
	node.children = new UctTreeNodes;
}

void SearchTree::expandNode(const Board & board, UctNodeBase & node, int color)
{
	if (!board.free.size())
		return;

	int children = 0;
	board.foreachFreePoint([&](coord idx)
	{
		if (!board.isValidNoSuicide({ idx, color }))
			return;

		node.children->nodes.emplace_back(idx); // Likely optimization point
	});
}
