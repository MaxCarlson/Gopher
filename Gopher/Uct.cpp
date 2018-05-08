#include "Uct.h"
#include "Board.h"
#include "MovePicker.h"
#include "Playout.h"
#include <math.h>
#include "Random.h"

static constexpr int TOTAL_PLAYOUTS = 18000;

// Expand a leaf node when it's been 
// visited this # of times
//static constexpr int EXPAND_FRACTION = TOTAL_PLAYOUTS / 200;

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

	// TODO: Clear tree before search or after!
	// Best time would be during opponents move
	tree.init(board, color);

	for (int i = 0; i < TOTAL_PLAYOUTS; ++i)
	{
		Board bb = board;

		playout(bb);

		if (i % (TOTAL_PLAYOUTS / 15) == 0 && i != 0)
			tree.printStatistics();
	}

	const coord bestMove = tree.getBestMove();

	// TODO: Need to add in heuristic for when it is best to pass!!!
	
	tree.afterSearch();

	return bestMove;
}

void Uct::playout(Board & board)
{
	static constexpr double HopelessRatio = 25.0;
	static constexpr int MaxGameLen = 400;

	static SmallVec<int, 100> moves;
	moves.clear();

	// Start walk from root
	int color = toPlay;

	walkTree(board, tree.root, moves, color);


	const int result = Playouts::playRandomGame(board, color, MaxGameLen, HopelessRatio);
	const bool win = isWin(result, toPlay, color);

	tree.recordSearchResults(moves, toPlay, win);
}

// Walk the tree from root using UCT
void Uct::walkTree(Board & board, UctNodeBase& root, SmallVec<int, 100>& moves, int& color)
{
	UctNodeBase* path = &root;
	while (path->expanded() && !path->isLeaf()) // TODO: One of these should be enough?
	{
		int bestIdx = 0;
		path = &chooseChild(*path, bestIdx);
		// Record the index of the move, we'll use it later to walk
		// the tree and record the results of this search
		moves.emplace_back(bestIdx);

		color = flipColor(color);
	}
	color = flipColor(color);

	++path->visits;

	// TODO: Only expand nodes once some threshold of playouts has been reached!?
	//if(++path->visits > EXPAND_FRACTION)
	tree.expandNode(board, *path, color);

	board.makeMove({ path->idx, color });
}

// Do not call this on a node with no children
// winrate + sqrt( (ln(parent.visits) ) / (5*n.nodevisits) )
UctNodeBase& Uct::chooseChild(UctNodeBase & node, int& bestIdx) const
{
	int idx = 0;
	double best = std::numeric_limits<double>::min();

	node.children->foreachChild(node.size, [&](const UctNodeBase& n)
	{
		double uct;

		// Give preference to unvisited nodes randomly
		if (n.visits == 0) 
			uct = 10000.0 + static_cast<double>(Random::fastRandom(10000));
		else
			uct = (static_cast<double>(n.wins)
				/ static_cast<double>(n.visits))
				+ std::sqrt(std::log(node.visits) / (EXPLORE_RATE * n.visits));

		if (uct > best)
		{
			bestIdx = idx;
			best = uct;
		}
		++idx;
	});

	// Increment visit counts
	++node.visits;

	return node.children->nodes[bestIdx];
}
