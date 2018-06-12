#include "Uct.h"
#include "Board.h"
#include "MovePicker.h"
#include "Playout.h"
#include <math.h>
#include "Random.h"

static constexpr int TOTAL_PLAYOUTS = 25000;
//static constexpr int TOTAL_PLAYOUTS = 1000; // DebugPlayouts


// Expand a leaf node when it's been 
// visited this # of times
static constexpr int EXPAND_AT = 8; 

static constexpr double HopelessCaptureMax = BoardSize2 / 2.5;


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

	auto start = Time::startTimer();

	// TODO: Add time limit support
	// TODO: Add in Pass to move tree ?

	for (int i = 0; i < TOTAL_PLAYOUTS; ++i)
	{
		Board bb = board;

		playout(bb);

		if (i % (TOTAL_PLAYOUTS / 10) == 0 && i != 0)
			tree.printBestLine();
	}

	std::cerr << "\n Search Time: " << Time::endTime<std::chrono::duration<double>>(start) << '\n';

	const coord bestMove = tree.getBestMove();

	tree.afterSearch();

	return bestMove;
}

void Uct::playout(Board & board)
{
	amafMap.clear();

	// Start walk from root
	int color = toPlay;

	walkTree(board, tree.root, color);

	const int result = Playouts::playRandomGame(board, amafMap, color, MAX_GAME_LENGTH, HopelessCaptureMax);	
	const bool win = isWin(result);
	
	tree.recordSearchResults(amafMap, color, win);
}

// Walk the tree from root using UCT
//
// TODO: Possible only generate  a single child at a time each time the leaf node is visited!!
void Uct::walkTree(Board & board, TreeNode& root, int& color)
{
	TreeNode* path = &root;
	while (!path->empty() && !path->isLeaf()) 
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


	// TODO: Only expand leaf nodes after n visits
	// Wil require additional checs when traversing tree

	if(path->uct.visits >= EXPAND_AT)
		tree.expandNode(board, *path, color);
}
