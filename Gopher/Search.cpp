#include "Search.h"
#include "Board.h"
#include "GameState.h"
#include "Net.h"

static constexpr int TOTAL_PLAYOUTS = 25000;

coord Search::search(const Board & board, GameState& state, int color)
{
	if (!root.isExpanded())
		root.expand();

	for (int i = 0; i < TOTAL_PLAYOUTS; ++i)
	{
		playout(board, state, &root, color);
	}

	// TODO: Add time based search instead of playout based!
	return coord();
}

void Search::playout(Board board, GameState & state, UctNode* const node, int color)
{
	
}

void Search::walkTree(Board & board, GameState & state, int & color)
{
}
