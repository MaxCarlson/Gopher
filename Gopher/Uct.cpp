#include "Uct.h"
#include "Board.h"
#include "MovePicker.h"
#include "Playout.h"
#include <math.h>
#include "Random.h"

//static constexpr int TOTAL_PLAYOUTS = 18000;
static constexpr int TOTAL_PLAYOUTS = 1000; // DebugPlayouts


// TODO: ? Expand a leaf node when it's been 
// visited this # of times
static constexpr int EXPAND_FRACTION = TOTAL_PLAYOUTS / 200;

/*
inline bool isWin(int result, int toPlay, int currentColor)
{
	return (toPlay == currentColor)
		? result > 0 
		: result < 0;
}
*/

inline bool isWin(int result)
{
	return result > 0;
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

		if (i % (TOTAL_PLAYOUTS / 10) == 0 && i != 0)
			tree.printBestLine();
	}

	const coord bestMove = tree.getBestMove();

	// TODO: Need to add in heuristic for when it is best to pass!!!
	
	tree.afterSearch();

	return bestMove;
}

void Uct::playout(Board & board)
{
	static constexpr double HopelessRatio = 25.0;

	amafMap.clear();

	// Start walk from root
	int color = toPlay;

	walkTree(board, tree.root, color);

	const int result = Playouts::playRandomGame(board, amafMap, color, MAX_GAME_LENGTH, HopelessRatio);	
	const bool win = isWin(result);
	
	tree.recordSearchResults(amafMap, color, win);
}

// Walk the tree from root using UCT
//
// TODO: Possible only generate  a single child at a time each time the leaf node is visited!!
void Uct::walkTree(Board & board, TreeNode& root, int& color)
{
	TreeNode* path = &root;
	while (path->expanded() && !path->isLeaf()) // TODO: One of these should be enough?
	{
		int bestIdx = 0;
		path = &RAVE::chooseChild(amafMap, *path, bestIdx);

		board.makeMove({ path->idx, color });

		// Record the index of the move, we'll use it later to walk
		// the tree and record the results of this search
		// TODO: Perhaps do these things both in the same function recursively?
		amafMap.addMoveInTree(path->idx, bestIdx);

		color = flipColor(color);
	}
	color = flipColor(color);

	tree.expandNode(board, *path, color);
}
