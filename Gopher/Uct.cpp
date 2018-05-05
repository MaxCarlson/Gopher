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
	static constexpr int playouts = 2000;

	// TODO: Clear tree before search or after!
	// Best time would be during opponents move
	tree.init(board, color);

	for (int i = 0; i < playouts; ++i)
	{
		Board bb = board;
		playout(bb);
	}

	const coord bestMove = tree.getBestMove();
	tree.writeOverTree();

	return bestMove;
}

void Uct::playout(Board & board)
{
	static constexpr double HopelessRatio = 25.0;
	static constexpr int MaxGameLen = 400;

	SmallVec<int, 100> moves;

	// Start walk from root
	int color = toPlay;

	walkTree(board, tree.root, moves, color);


	const int result = Playouts::playRandomGame(board, color, MaxGameLen, HopelessRatio);
	const bool win = isWin(result, toPlay, color);

	tree.recordSearchResults(moves, toPlay, win);
}

// Walk the tree from root using UCT
void Uct::walkTree(Board & board, UctNodeBase& node, SmallVec<int, 100>& moves, int& color)
{
	UctNodeBase* path = &node;
	while (path->expanded() && !path->isLeaf())
	{
		int bestIdx = 0;
		path = &chooseChild(*path, bestIdx);
		moves.emplace_back(bestIdx);

		color = flipColor(color);
	}
	color = flipColor(color);


	++path->visits;
	tree.expandNode(board, *path, color);

	moves.emplace_back(path->idx);
	board.makeMove({ path->idx, color });
}

// Do not call this on a node with no children
// winrate + sqrt( (ln(parent.visits) ) / (5*n.nodevisits) )
UctNodeBase& Uct::chooseChild(UctNodeBase & node, int& bestIdx) const
{
	int idx = 0;
	double best = std::numeric_limits<double>::min();

	for (const auto& n : node.children->nodes)
	{
		if (n.visits == 0) // TODO: Need to randomize picking unexplored node instead of picking first TOP PRIORITY
		{
			bestIdx = idx;
			break;
		}

		double uct = (static_cast<double>(n.wins) 
					/ static_cast<double>(n.visits)) 
			+ std::sqrt(std::log(node.visits) / (EXPLORE_RATE * n.visits));

		if (uct > best)
		{
			bestIdx = idx;
			best = uct;
		}
		++idx;
	}

	// Increment visit counts
	++node.visits;

	return node.children->nodes[bestIdx];
}
