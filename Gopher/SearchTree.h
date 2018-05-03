#pragma once
#include <Containers\SmallVec.h>
class Board;

struct UctNodeBase;
constexpr int AVG_CHILDREN = 1;
using ChildStorage = SmallVec<UctNodeBase, AVG_CHILDREN>;

struct UctTreeNodes
{
	ChildStorage nodes;
};

struct UctNodeBase
{
	UctNodeBase() = default;
	UctNodeBase(coord idx) : idx(idx) {}

	int wins = 0;
	int visits = 0;
	coord idx;

	UctTreeNodes* children;

	size_t size() const { return children ? children->nodes.size() : 0; }
};

class SearchTree
{
	void allocateChildren(UctNodeBase& node);

public:
	
	UctNodeBase root;


	void expandNode(const Board& board, UctNodeBase& node, int color);
};

