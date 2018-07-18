#include "Search.h"
#include "UctNode.h"
#include "Board.h"
#include "GameState.h"
#include "Net.h"

static constexpr int TOTAL_PLAYOUTS = 361 * 1.5;// 25000;

coord Search::search(const Board & board, GameState& state, int color)
{
	Tree::initRoot(board, state, color);

	for (int i = 0; i < TOTAL_PLAYOUTS; ++i)
	{
		Board b = board;
		playout(b, state, Tree::getRoot(), 0, color);

		if (i % (TOTAL_PLAYOUTS / 10) == 0)
			Tree::printStats(color);
	}

	Tree::printNodeInfo(&Tree::getRoot());

	// TODO: Add time based search instead of playout based!
	auto& best = Tree::findBestMove(&Tree::getRoot());
	coord idx = best.idx;

	Tree::switchRoot(best);
	return idx;
}

float Search::playout(Board& board, GameState & state, UctNode& node, int depth, int color)
{
	++depth;

	// TODO: Implement stop if Passes >= 2
	// TODO: Add output 362 for passes to Net
	// TODO: Need to limit expansions somehow as search grows larger
	float value;
	const auto isRoot = depth == 1;

	// TODO: Need to handle end game conditions better
	if (!node.isExpanded())
	{
		NetResult netResult = Net::run(state, color);

		node.expand(state, board, netResult, color);

		value = node.getNetEval(color);
	}
	else if (!node.empty())
	{
		auto& bestChild = node.selectChild(color, isRoot);

		board.makeMove({ bestChild.idx, color });
		state.makeMove(board);

		value = playout(board, state, bestChild, depth, flipColor(color));

		// Roll back the board state (Not the actual board though)
		// That will be undone at the very end
		state.popState();
	}
	// How should this branch(Leaf but previously expanded) be handled?
	// Return recorded score?
	else
		value = node.getEval(color);

	node.update(value);

	return value;
}
