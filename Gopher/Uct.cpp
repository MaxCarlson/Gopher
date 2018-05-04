#include "Uct.h"
#include "Board.h"
#include "MovePicker.h"
#include <math.h>

static constexpr double EXPLORE_RATE = 5.0;

coord Uct::search(const Board & board, int color)
{
	static constexpr int playouts = 40000;

	// TODO: Clear tree before search or after!
	// Best time would be during opponents move
	tree.init(board, color);

	for (int i = 0; i < playouts; ++i)
	{
		Board bb = board;
		playout(bb);
	}

	return 0;
}

void Uct::playout(Board & board)
{

	walkTree(board);

}

// Walk the tree from root using UCT
void Uct::walkTree(Board & board)
{
	
	// Start walk from root
	auto& nextNode = chooseChild(tree.root);

	while (nextNode.expanded())
	{
		nextNode = chooseChild(nextNode);
	}
}

// Do not call this on a node with no children
// winrate + sqrt( (ln(parent.visits) ) / (5*n.nodevisits) )
UctNodeBase& Uct::chooseChild(UctNodeBase & node) const
{
	int idx = 0;
	int bestIdx = 0;
	double best = -100000.0;
	for (const auto& n : node.children->nodes)
	{
		double uct = n.wins + std::sqrt(std::log(node.visits) / (EXPLORE_RATE * n.visits));

		if (uct > best)
		{
			bestIdx = idx;
			best = uct;
		}
		++idx;
	}

	// Increment visit counts
	++node.visits;
	++node.children->nodes[bestIdx].visits;

	return node.children->nodes[bestIdx];
}
