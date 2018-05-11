#include "Amaf.h"
#include "SearchTree.h"
#include "Random.h"
#include <array>


namespace RAVE
{
	void updateTree(const AmafMap & moves, TreeNode * node, int toMove, int color, bool isWin)
	{
		if (toMove != color)
			isWin = !isWin;

		// Build a map of the moves made during playout
		// so we can quickly look them up as well as 
		// the order in which they were played
		static constexpr int EMPTY = std::numeric_limits<int>::max();

		std::array<int, BoardRealSize2 + 1> movesTmp;
		movesTmp.fill(EMPTY);

		// So Pass as -1 isn't out of bounds
		int* movesByIdx = &movesTmp[1];

		// Add moves by index backwards so 
		// newer moves overwrite older ones 
		// TODO: Should i >= 0 be i >= moves.movesInTree.size() ?
		for (int i = moves.moves.size() - 1; i >= 0; --i)
			movesByIdx[moves.moves[i]] = i;

		// Traverse down the tree from the root,
		// updating node win statistics and AMAF values
		for (int mIdx = 0; mIdx < moves.movesInTree.size(); ++mIdx)
		{
			node = &node->children->nodes[moves.movesInTree[mIdx]];

			// Update the nodes win statistics
			node->wins += isWin;

			// Update AMAF values
			node->children->foreachChild(node->size, [&](TreeNode &child)
			{
				const int firstPlayed = movesByIdx[child.idx];

				if (firstPlayed == EMPTY)
					return;

				// Don't use moves not played by this nodes color
				// TODO: Make sure this matches up
				const int dist = firstPlayed - mIdx;
				if (dist & 1)
					return;

				// TODO: Later moves are worth less
				// TODO: Make sure amafMap during playout doesn't 
				// overwrite moves closer to the root with older moves

				++child.amaf.visits;
				child.amaf.wins += isWin;
			});

			isWin = !isWin;
		}
	}

	static constexpr double RAVE_EQ = 3000.0;
	static constexpr double UCT_EXPLORE = 0.5;

	// AMAF playouts, AMAF value, Node playouts
	// Idea is to start out using RAVE and as visits increase,
	// start letting MonteCarlo UCT have a higher weight
	inline double getBeta(double amafP, double amafV, double uctP)
	{
		/*
		if (r.playouts) {
			 At the beginning, beta is at 1 and RAVE is used.
			* At b->equiv_rate, beta is at 1/3 and gets steeper on. 
			floating_t beta;
			if (b->sylvain_rave) {
				beta = r.playouts / (r.playouts + n.playouts + n.playouts * r.playouts / b->equiv_rave);
		*/
		if (amafP)
			return amafP / (amafP + uctP + uctP * amafP / RAVE_EQ);
		else
			return amafV;
	}

	inline double uctRave(const TreeNode& child)
	{
		double val;
		if(child.visits == 0)
			val = 10000.0 + static_cast<double>(Random::fastRandom(10000));
		else
		{
			// TODO: These should be cached and updated in updateTree?
			double uct = static_cast<double>(child.wins) / static_cast<double>(child.visits);
			double amaf = static_cast<double>(child.amaf.wins) / static_cast<double>(child.amaf.visits);

			const double beta = getBeta(child.amaf.visits, amaf, child.visits);
			val = (1 - beta) * uct + (beta * amaf);
		}
		return val;
	}

	TreeNode & chooseChild(const AmafMap & moves, TreeNode & node, int& bestIdx)
	{
		int idx = 0;
		double best = std::numeric_limits<double>::min();

		node.children->foreachChild(node.size, [&](const TreeNode& c)
		{
			double uct;

			// Give preference to unvisited nodes randomly
			if (c.visits == 0)
				uct = 10000.0 + static_cast<double>(Random::fastRandom(10000));
			else
				uct = uctRave(c) + UCT_EXPLORE 
					* std::sqrt(std::log(node.visits) / c.visits);
				/*
				uct = (static_cast<double>(c.wins)
					/ static_cast<double>(c.visits))
				+ UCT_K * std::sqrt(std::log(node.visits) / c.visits);
				*/

			if (uct > best)
			{
				bestIdx = idx;
				best = uct;
			}
			++idx;
		});

		// Increment visit counts
		++node.visits;

		return node.children->nodes[bestIdx];
	}
}

