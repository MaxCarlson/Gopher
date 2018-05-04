#pragma once
#include "SearchTree.h"

struct Board;
using coord = int;
struct UctNodeBase;
template<class T, int, class Alloc>
class SmallVec;

class Uct
{
public:

	coord search(const Board& board, int color);

private:
	int toPlay;
	SearchTree tree;

	void playout(Board& board);
	void walkTree(Board& board, UctNodeBase& node, SmallVec<int, 100>& moves, int& color);

	// Find the best child of the node based on UCT's algo
	UctNodeBase& chooseChild(UctNodeBase& node, int& bestIdx) const;

};

inline Uct uct;

