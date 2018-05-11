#pragma once
#include "defines.h"
#include "Containers\SmallVec.h"

struct TreeNode;

struct AmafMap
{
	SmallVec<coord, MAX_GAME_LENGTH> moves;
	SmallVec<int, 100> movesInTree;

	void addMove(const coord idx)
	{
		moves.emplace_back(idx);
	}

	void addMoveInTree(const coord idx)
	{
		moves.emplace_back(idx);
		movesInTree.emplace_back(idx);
	}

	void clear()
	{
		moves.clear();
		movesInTree.clear();
	}
};

namespace RAVE
{
	void updateTree(const AmafMap& moves, TreeNode* root, int toMove, int color, bool isWin);
}

