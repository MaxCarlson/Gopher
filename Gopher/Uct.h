#pragma once
#include "SearchTree.h"

struct Board;
using coord = int;
struct UctNodeBase;

class Uct
{
public:

	coord search(const Board& board, int color);

private:

	SearchTree tree;

	void playout(Board& board);
	void walkTree(Board& board);

	// Find the best child of the node based on UCT's algo
	UctNodeBase& chooseChild(UctNodeBase& node) const;

};

inline Uct uct;

