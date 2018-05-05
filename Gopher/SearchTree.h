#pragma once
#include <Containers\SmallVec.h>

struct Board;


constexpr int AVG_CHILDREN = 5;

using coord = int;
struct UctTreeNodes;

struct UctNodeBase
{
	UctNodeBase() = default;
	UctNodeBase(coord idx) : idx(idx) {}

	int wins = 0;
	int visits = 0;
	coord idx;

	// Keep track of how many children
	// we actually have as the small vec in children
	// likely has more spaces filled that we haven't deallocated
	short size = 0;
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

class SearchTree
{
	void allocateChildren(UctNodeBase& node);

public:

	int baseColor;
	UctNodeBase root;

	void init(const Board& board, int color);
	coord getBestMove() const;

	void writeOverTree();

	void expandNode(const Board& board, UctNodeBase& node, int color);

	// Walk the tree from the root and record the results of the playout
	// visits have already been incremeneted
	void recordSearchResults(SmallVec<int, 100>& moves, int color, bool isWin);
};


