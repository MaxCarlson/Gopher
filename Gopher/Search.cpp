#include "Search.h"
#include "Board.h"
#include "GameState.h"
#include "Net.h"

static constexpr int MAX_DEPTH = 250;
static constexpr int TOTAL_PLAYOUTS = 25000;

coord Search::search(const Board & board, GameState& state, int color)
{
	if (!root.isExpanded())
		root.expand(state, board, color);

	for (int i = 0; i < TOTAL_PLAYOUTS; ++i)
	{
		Board b = board;
		playout(b, state, &root, 0, color);
	}

	// TODO: Add time based search instead of playout based!
	return coord();
}

int Search::playout(Board& board, GameState & state, UctNode* const node, int depth, int color)
{
	++depth;
	++node->visits;

	// TODO: Implement stop if Passes >= 2
	// TODO: Add output 362 for passes to Net
	// TODO: Add dynamic depth increases with a limit early on, lossening later in the search
	// mainly so we can get more exploration
	if (depth >= MAX_DEPTH)
		return board.scoreFast();

	// TODO: Need to limit expansions somehow as search grows larger
	// TODO: Dynamically lower total nodes expanded when expanding later in search
	if (!node->isExpanded())
		node->expand(state, board, color);

	if (node->children.size() < 15)
	{
		board.printBoard();
		node->children.clear();
		node->expand(state, board, color);
	}

	int result = 0;

	if (!node->children.empty())
	{
		const auto& bestChild = node->selectChild(color);

		board.makeMove({ bestChild->idx, color });
		state.makeMove(board);

		result = playout(board, state, bestChild, depth, flipColor(color));

		// Roll back the board state (Not the actual board though)
		// That will be undone at the very end
		state.popState();
	}
	else
		result = board.scoreFast();

	node->scoreNode(result, color);

	return result;
}

void Search::walkTree(Board & board, GameState & state, int & color)
{
}
