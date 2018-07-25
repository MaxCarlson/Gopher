#include "Search.h"
#include "UctNode.h"
#include "Board.h"
#include "GameState.h"
#include "Net.h"

static constexpr int TOTAL_PLAYOUTS = 361 * 5;


coord Search::search(const Board & board, GameState& state, int color)
{
	Tree::initRoot(board, state, color);

	for (int i = 0; i < TOTAL_PLAYOUTS; ++i)
	{
		Board b = board;
		playout(b, state, Tree::getRoot(), 0, color);

		// TODO: Config option for # of printouts during search
		if (i % (TOTAL_PLAYOUTS / 10) == 0)
			Tree::printStats(color);
	}

	// Debugging. Look at spread of search through tree
	Tree::printNodeInfo(&Tree::getRoot());

	// TODO: Add time based search instead of playout based!
	// TODO: Add in passing!
	coord idx;
	auto* best = Tree::findBestMove(&Tree::getRoot(), color);

	if (resignOrPass(best, idx, color))
		;
	else if (best)
	{
		idx = best->idx;
		Tree::updateRoot(*best);
	}

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

// TODO: Make passing more comprehensive. 
// TODO: Tune Resign
bool Search::resignOrPass(const UctNode* best, coord& idx, int color) const
{
	auto rOrp		 = false;
	const auto& root = Tree::getRoot();
	static constexpr auto resignThresh = 0.05;

	// Pass if there are no moves avalible
	if (!best)
	{
		rOrp	= true;
		idx		= Pass;
	}

	// Resign if win chance gets too low.
	// This is important for faster play testing of new versions
	if (root.getEval(color) < resignThresh)
	{
		rOrp	= true;
		idx		= Resign;
	}

	return rOrp;
}
