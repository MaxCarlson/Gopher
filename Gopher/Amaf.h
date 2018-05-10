#pragma once
#include "defines.h"
#include "Containers\SmallVec.h"

struct TreeNode;

struct AmafMap
{
	SmallVec<coord, MAX_GAME_LENGTH> moves;

	// Denotes where we start counting from in some places,
	// and where we stop in others since moves
	// holds the move indices made during tree traversal as well
	int root = 0;

	void addMove(const coord idx)
	{
		moves.emplace_back(idx);
	}

	void addMoveInTree(const coord idx)
	{
		moves.emplace_back(idx);
		++root;
	}

	void clear()
	{
		root = 0;
		moves.clear();
	}
};

namespace RAVE
{
	void updateTree(const AmafMap& moves, TreeNode* root, int toMove, int color, bool isWin);
}

