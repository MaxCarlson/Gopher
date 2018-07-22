#include "Search.h"
#include "UctNode.h"
#include "Board.h"
#include "GameState.h"
#include "Net.h"

static constexpr int TOTAL_PLAYOUTS = 10;//361 * 1;


// TODO Sometimes search picks literally the worst move by policy standards
// (But it's a move that's been visited a ton. Why was it visited?)
//
// TODO: Find out why it seems to evaluate an awful position for black
// as a 100% win chance!!

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
	auto& best	= Tree::findBestMove(&Tree::getRoot(), color);
	coord idx	= best.idx;
	Tree::updateRoot(best);

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
		NetResult netResult = Net::inference(state, color);

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
		state.popState();
	}
	// How should this branch(empty leaf but previously expanded) be handled?
	// Return recorded score?
	else
		value = node.getEval(color);

	node.update(value);

	return value;
}
