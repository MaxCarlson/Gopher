#pragma once
#include "SearchTree.h"
#include "Amaf.h"

struct Board;
using coord = int;
struct TreeNode;
template<class T, int, class Alloc>
class SmallVec;

class Uct
{
public:

	coord search(const Board& board, int color);

private:
	int toPlay;
	SearchTree tree;
	AmafMap amafMap;

	void playout(Board& board);
	void walkTree(Board& board, TreeNode& root, int& color);
};

inline Uct uct;

