#pragma once
#include "defines.h"
#include "Containers\SmallVec.h"

struct TreeNode;

struct AmafMap
{
	SmallVec<int, 100> movesInTree;
	SmallVec<coord, MAX_GAME_LENGTH> moves;

	void addMove(const coord idx)
	{
		moves.emplace_back(idx);
	}

	void addMoveInTree(const coord moveIdx, const int arrayIdx)
	{
		moves.emplace_back(moveIdx);
		movesInTree.emplace_back(arrayIdx);
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

	TreeNode& chooseChild(const AmafMap& moves, TreeNode& root, int& bestIdx);
}

