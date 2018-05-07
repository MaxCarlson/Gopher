#pragma once
#include <Containers\SmallVec.h>

struct Board;


constexpr int AVG_CHILDREN = 1;

using coord = int;
struct UctTreeNodes;

struct UctNodeBase
{
	UctNodeBase() = default;
	UctNodeBase(coord idx) : idx(idx) {}

	int wins = 0;
	int visits = 0;		// TODO: enable_if stuff based on MAX_VISITS possible to reduce size of this ?
	coord idx;

	// Keep track of how many children
	// we actually have as the small vec in children
	// likely has more spaces filled that we haven't deallocated
	int size = 0; // TODO: Right now due to aligment this doesn't make a difference in size if it and idx are both shorts or not, revisit

	UctTreeNodes* children = nullptr;

	bool expanded() const { return visits; }
	bool isLeaf() const;
};

struct UctTreeNodes
{
	SmallVec<UctNodeBase, AVG_CHILDREN> nodes;

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
	void allocateChildren(UctNodeBase& node);

public:

	int baseColor;
	UctNodeBase root;

	void init(const Board& board, int color);
	void afterSearch();

	SearchStatistics getStatistics() const;
	coord getBestMove() const;
	void printStatistics() const;

	void writeOverBranch(UctNodeBase& root);


	void expandNode(const Board& board, UctNodeBase& node, int color);

	// Walk the tree from the root and record the results of the playout
	// visits have already been incremeneted
	void recordSearchResults(SmallVec<int, 100>& moves, int color, bool isWin);
};


