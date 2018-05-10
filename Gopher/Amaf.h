#pragma once
#include "defines.h"
#include "Containers\SmallVec.h"

struct AmafMap
{
	SmallVec<coord, MAX_GAME_LENGTH> moves;

	// Denotes where we start counting from,
	// since moves holds the moves made during tree 
	// traversal as well
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

