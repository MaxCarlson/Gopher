#pragma once
#include <vector>
#include <Containers\SmallVec.h>

using coord = int;
struct UctTreeNodes;

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
	~TreeNode();

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

	template<class F>
	void foreachChild(F&& f);

	template<class F>
	void foreachChildBreak(F&& f);
};

struct UctTreeNodes
{
	//SmallVec<TreeNode, AVG_CHILDREN> nodes;
	std::vector<TreeNode> nodes;
};

template<class F>
void TreeNode::foreachChild(F&& f)
{
	for (int i = 0; i < size; ++i)
		f(children->nodes[i]);
}

// Takes a size to iterate through
// a llambda [&](UctNodeBase&, bool&)
// set stop to false to bool iterating
template<class F>
void TreeNode::foreachChildBreak(F&& f)
{
	bool stop = false;
	for (int i = 0; i < size; ++i)
	{
		f(children->nodes[i], stop);
		if (stop)
			return;
	}
}