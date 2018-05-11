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

		// Add moves by index backwards so newer moves overwrite older ones. 
		// Don't add moves that are in the search tree itself
		for (int i = moves.moves.size() - 1; i >= 0; --i)
			movesByIdx[moves.moves[i]] = i + moves.movesInTree.size();

		
		// Traverse down the tree from the root,
		// updating node win statistics and AMAF values
		++node->uct.visits;
		node->uct.wins += isWin;
		for (int mIdx = 0; mIdx < moves.movesInTree.size(); ++mIdx)
		{
			node = &node->children->nodes[moves.movesInTree[mIdx]];

			// Update the nodes win and visited statistics
			node->uct.wins += isWin;
			++node->uct.visits;

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
				child.amaf.wins += !isWin;
			});

			isWin = !isWin;
		}
	}

	static constexpr double RAVE_EQ = 3000.0;
	static constexpr double UCT_EXPLORE = 0.5;

	// Idea is to start out using RAVE and as visits increase,
	// start letting MonteCarlo UCT have a higher weight
	// AMAF playouts, AMAF value, Node playouts
	inline double getBeta(double amafP, double amafV, double uctP)
	{
		return amafP / (amafP + uctP + uctP * amafP / RAVE_EQ);
	}

	inline double uctRave(const TreeNode& child)
	{
		double val;

		// TODO: These should be cached and updated in updateTree?
		const double uct = static_cast<double>(child.uct.wins) / static_cast<double>(child.uct.visits);

		if (child.amaf.visits)
		{
			// TODO: These should be cached and updated in updateTree?
			const double amaf = static_cast<double>(child.amaf.wins) / static_cast<double>(child.amaf.visits);

			const double beta = getBeta(child.amaf.visits, amaf, child.uct.visits);
			val = (1.0 - beta) * uct + (beta * amaf);
		}
		else
			val = uct;
		
		return val;
	}

	TreeNode & chooseChild(const AmafMap & moves, TreeNode & node, int& bestIdx)
	{
		int idx = 0;
		double best = std::numeric_limits<double>::min();

		node.children->foreachChild(node.size, [&](const TreeNode& c)
		{
			double val;

			// Give preference to unvisited nodes randomly
			if (c.uct.visits == 0)
				val = 10000.0 + static_cast<double>(Random::fastRandom(10000));
			else
				val = uctRave(c) + UCT_EXPLORE 
					* std::sqrt(std::log(node.uct.visits) / c.uct.visits);

				/* // Old straight UCT
				uct = (static_cast<double>(c.wins)
					/ static_cast<double>(c.visits))
				+ UCT_EXPLORE * std::sqrt(std::log(node.visits) / c.visits);
				*/

			if (val > best)
			{
				bestIdx = idx;
				best = val;
			}
			++idx;
		});

		return node.children->nodes[bestIdx];
	}
}

