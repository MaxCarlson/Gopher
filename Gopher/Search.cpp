#include "Search.h"
#include "UctNode.h"
#include "Board.h"
#include "GameState.h"
#include "Net.h"
#include "Random.h"
#include "Options.h"
#include <iomanip>

static constexpr int EXPAND_THRESHOLD	= 0;

coord Search::search(const Board & board, GameState& state, int color)
{
	auto start = Time::startTimer();
	Tree::initRoot(board, state, color);

	//board.printBoard();
	//state.printStates();
	//NetInput inp(state, color);
	//inp.printSlices(state);
	//options.maxPlayouts = 7000;
	
	for (int i = 0; i < options.maxPlayouts; ++i)
	{
		Board b = board;
		playout(b, state, Tree::getRoot(), 0, color);

		// TODO: Config option for printing frequency
		if (i % (options.maxPlayouts / 10) == 0)
			Tree::printStats(color);
	}

	// Debugging. Look at spread of search through tree
	Tree::printNodeInfo(&Tree::getRoot(), color);

	// TODO: Add time based search instead of playout based!
	// TODO: Add in better passing mechanisms!
	coord idx;
	auto* best = Tree::findBestMove(&Tree::getRoot(), color);

	// When performing validation we don't want the engine to play
	// the same games against itself over and over. Add some randomness to early moves
	if (options.validation && options.rngMovesNumber > state.moveCount)
		moveNoise(best, color);

	if (resignOrPass(state, best, idx, color))
		;
	else if (best)
	{
		idx = best->idx;
		Tree::updateRoot(*best);
	}

	auto end = Time::endTime<std::chrono::duration<double>>(start);
	std::cerr << "Search Time: " << std::fixed << std::setprecision(1) << end << "s\n";

	return idx;
}

float Search::playout(Board& board, GameState & state, UctNode& node, int depth, int color)
{
	++depth;

	// TODO: Implement stop if Passes >= 2
	// TODO: Add output 362 for passes to Net
	// TODO: Need to limit expansions somehow as search grows larger
	// TODO: Need to handle end game conditions better
	float value;
	const auto isRoot = depth == 1;

	if (!node.isExpanded() && node.visits >= EXPAND_THRESHOLD)
	{
		NetResult netResult = Net::inference(state, color);

		node.expand(state, board, netResult, color);

		// Net values are stored from blacks perspective
		value = node.getNetEval(BLACK);
	}
	else if (!node.empty())
	{
		auto* bestChild = node.selectChild(color, isRoot);

		board.makeMove({ bestChild->idx, color });
		state.makeMove(board);

		value = playout(board, state, *bestChild, depth, flipColor(color));

		// Roll back the board state (Not the actual board though)
		state.popState();
	}
	// How should this branch(empty leaf but previously expanded) be handled?
	// Return recorded score?
	else
		value = node.getNetEval(BLACK);

	node.update(value);

	return value;
}

// TODO: Make passing more comprehensive. 
// TODO: Tune Resign
bool Search::resignOrPass(const GameState& state, const UctNode* best, coord& idx, int color) const
{
	auto rOrp		 = false;
	const auto& root = Tree::getRoot();

	// Pass if there are no moves avalible
	// Or if we're past the max moves allowed for validation
	if (!best || (options.validation 
		&& options.valMaxMoves < state.moveCount))
	{
		rOrp	= true;
		idx		= Pass;
	}

	// Resign if win chance gets too low.
	// This is important for faster play testing of new versions
	if (root.getEval(color) < options.resignThresh)
	{
		rOrp	= true;
		idx		= Resign;
	}

	return rOrp;
}

void Search::moveNoise(UctNode*& best, int color) const
{
	auto roll = Random::fastRandom(Tree::getRoot().visits);

	// Build map of moves visited during the search
	int vcounter = 0;
	std::vector<std::pair<int, UctNode*>> visits;
	for (auto& child : *Tree::getRoot().children)
	{
		if (child.visits)
		{
			vcounter += child.visits;
			visits.emplace_back(vcounter, &child);
		}
	}

	// Pick moves proportionally based on
	// how many visits they have
	for (auto& child : visits)
		if (child.first >= roll)
		{
			best = child.second;
			break;
		}
}
