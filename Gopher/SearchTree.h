#pragma once
#include "TreeNode.h"
struct Board;

constexpr int AVG_CHILDREN = 1;

using coord = int;
struct AmafMap;

struct SearchStatistics
{
	coord idx; // This is only here to easily represent Resign and pass
	TreeNode *best;
	double winRate;
};

class SearchTree
{
	void allocateChildren(TreeNode& node);

public:

	int rootColor;
	TreeNode root;

	void init(const Board& board, int color);
	void afterSearch();

	SearchStatistics getStatistics(const TreeNode& root, bool resignOnWinChance) const;
	coord getBestMove() const;
	void printBestLine() const;

	void expandNode(const Board& board, TreeNode& node, int color);


	void writeOverBranch(TreeNode& root);
	// Prune the tree of nodes that are no longer needed
	int pruneTree(TreeNode& root);

	// Walk the tree from the root and record the results of the playout
	// visits have already been incremeneted
	void recordSearchResults(const AmafMap& moves, int color, bool isWin);
};


