#include "Uct.h"
#include "Board.h"
#include "MovePicker.h"
#include "SearchTree.h"
#include <math.h>

static constexpr float EXPLORE_RATE = 5.0;

coord Uct::search(const Board & board, int color)
{
	Board bb = board;

	static constexpr int playouts = 40000;

	

	return 0;
}

// Do not call this on a node with no children
// winrate + sqrt( (ln(parent.visits) ) / (5*n.nodevisits) )
UctNodeBase& Uct::chooseChild(const UctNodeBase & node) const
{
	int idx = 0;
	int bestIdx = 0;
	float best = 0.0;
	for (const auto& n : node.children->nodes)
	{
		float uct = n.wins + std::sqrt(std::log(node.visits) / (EXPLORE_RATE * n.visits));

		if (uct > best)
		{
			bestIdx = idx;
			best = uct;
		}
		++idx;
	}

	return node.children->nodes[bestIdx];
}
