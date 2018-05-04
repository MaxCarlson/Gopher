#include "Uct.h"
#include "Board.h"
#include "MovePicker.h"
#include "Playout.h"
#include <math.h>

static constexpr double EXPLORE_RATE = 5.0;

inline bool isWin(int result, int toPlay, int currentColor)
{
	return (toPlay == currentColor)
		? result > 0 
		: result < 0;
}

coord Uct::search(const Board & board, int color)
{
	toPlay = color;
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
	static constexpr double HopelessRatio = 25.0;
	static constexpr int MaxGameLen = 400;

	SmallVec<int, 100> moves;

	// Start walk from root
	int color = toPlay;
	auto& node = tree.root;

	walkTree(board, node, moves, color);


	const int result = Playouts::playRandomGame(board, color, MaxGameLen, HopelessRatio);
	const bool win = isWin(result, toPlay, color);

	tree.recordSearchResults(moves, toPlay, win);
}

// Walk the tree from root using UCT
void Uct::walkTree(Board & board, UctNodeBase& node, SmallVec<int, 100>& moves, int& color)
{
	while (node.expanded() && !node.isLeaf())
	{
		int bestIdx = 0;
		node = chooseChild(node, bestIdx);
		moves.emplace_back(bestIdx);

		color = flipColor(color);
	}

	tree.expandNode(board, node, color);

	const Move m = MovePicker::pickMove(board, color);

	// TODO: Should this move be recorded ?
	//moves.emplace_back(m.idx);
	
	board.makeMove(m);
}

// Do not call this on a node with no children
// winrate + sqrt( (ln(parent.visits) ) / (5*n.nodevisits) )
UctNodeBase& Uct::chooseChild(UctNodeBase & node, int& bestIdx) const
{
	int idx = 0;
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
