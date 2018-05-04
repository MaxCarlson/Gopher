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

	UctTreeNodes* children;

	size_t size() const;
	bool expanded() const { return visits; }
};

struct UctTreeNodes
{
	SmallVec<UctNodeBase, AVG_CHILDREN> nodes;
};

class SearchTree
{
	void allocateChildren(UctNodeBase& node);

public:

	int baseColor;
	UctNodeBase root;

	void init(const Board& board, int color);

	void expandNode(const Board& board, UctNodeBase& node, int color);


};


