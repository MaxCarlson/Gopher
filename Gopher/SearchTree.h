#pragma once
#include <Containers\SmallVec.h>

struct Board;


constexpr int AVG_CHILDREN = 1;

using coord = int;
struct UctTreeNodes;
struct AmafMap;

struct MoveStat
{
	int wins = 0;
	int visits = 0;

	void clear() { wins = visits = 0; }
};

struct TreeNode
{
	TreeNode() = default;
	TreeNode(coord idx) : idx(idx) {}

	coord idx;

	// Keep track of the actual size we hold,
	// children likely has more total nodes than we have child nodes
	int size = 0;

	MoveStat amaf;
	MoveStat uct;

	UctTreeNodes* children = nullptr;

	bool expanded() const { return size; } // TODO: This should not be used with size
	bool isLeaf() const;
	void clearStats();
};

struct UctTreeNodes
{
	SmallVec<TreeNode, AVG_CHILDREN> nodes;

	template<class F>
	void foreachChild(int size, F&& f)
	{
		for (int i = 0; i < size; ++i)
			f(nodes[i]);
	}

	// Takes a size to iterate through
	// a llambda [&](UctNodeBase&, bool&)
	// set stop to false to bool iterating
	template<class F>
	void foreachChildBreak(int size, F&& f)
	{
		bool stop = false;
		for (int i = 0; i < size; ++i)
		{
			f(nodes[i], stop);
			if (stop)
				return;
		}
	}
};

struct SearchStatistics
{
	coord bestIdx;
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

	SearchStatistics getStatistics() const;
	coord getBestMove() const;
	void printStatistics() const;

	void writeOverBranch(TreeNode& root);


	void expandNode(const Board& board, TreeNode& node, int color);

	// Walk the tree from the root and record the results of the playout
	// visits have already been incremeneted
	void recordSearchResults(const AmafMap& moves, int color, bool isWin);
};


