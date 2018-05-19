#include "Amaf.h"
#include "Random.h"
#include "TreeNode.h"
#include <array>


namespace RAVE
{
	void updateTree(const AmafMap & moves, TreeNode * node, int toMove, int color, bool isWin)
	{
		if (!moves.movesInTree.size())
			return;

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

		const int gameLength = moves.moves.size();

		// Add moves by index backwards so newer moves overwrite older ones. 
		// Don't add moves that are in the search tree itself
		for (int i = gameLength - 1; i >= moves.movesInTree.size(); --i)
			movesByIdx[moves.moves[i]] = i;

		
		// Traverse down the tree from the root,
		// updating node win statistics and AMAF values
		node->uct.addData(1, isWin);

		for (int mIdx = 0; mIdx < moves.movesInTree.size(); ++mIdx)
		{
			node = &node->children->nodes[moves.movesInTree[mIdx]];

			// Update the nodes win and visited statistics
			node->uct.addData(1, isWin);

			// Update AMAF values
			node->foreachChild([&](TreeNode &child)
			{
				const int firstPlayed = movesByIdx[child.idx];

				if (firstPlayed == EMPTY)
					return;

				// Don't use moves not played by this nodes color
				const int dist = firstPlayed - (mIdx + 1); // + 0 for RevParity
				if (dist & 1)
					return;

				// Earlier moves are worth more
				static constexpr double RAVE_DIST = 3.0;
				const int weight = 1 + (RAVE_DIST * (gameLength - firstPlayed) / (gameLength - mIdx));
				//int weight = 1; // Only for NoDistWeight

				child.amaf.addData(weight, !isWin * weight); 
			});

			isWin = !isWin;
		}
	}

	static constexpr double RAVE_EQ = 450.0; 
	static constexpr double UCT_EXPLORE = 0.35; 

	// Idea is to start out using RAVE and as visits increase,
	// start letting MonteCarlo UCT have a higher weight
	// AMAF playouts, AMAF value, Node playouts
	/*
	inline double getBeta(double simsAmaf, double simsUct)
	{
		return simsAmaf / (simsAmaf + simsUct + simsAmaf * simsUct / RAVE_EQ);
	}
	*/

	/*
	inline double getBeta(double simsAmaf, double simsUct)
	{
		return 0.3;
	}
	//*/

	/*
	// Give more weight to UCT as simsAMAF increases
	// 100% weight to UCT when simsAMAF goes over USE_UCT
	inline double getBeta(double simsAmaf, double simsUct) // This is currently the best
	{
		static constexpr double USE_UCT = 100;

		return std::max(0.0, (USE_UCT - simsAmaf) / USE_UCT);
	}
	*/

	///*
	inline double getBeta(double simsAmaf, double simsUct)
	{
		static constexpr double RAVE_BIAS = 0.1;
		
		return simsAmaf / (simsUct + simsAmaf + (4 * simsUct * simsAmaf * RAVE_BIAS));
	}
	//*/

	double uctRave(const TreeNode& child)
	{
		double val;

		/*/
		double a = getBeta(10, 5);
		double aa = getBeta(5, 15);
		double aaa = getBeta(80, 2);
		double aaaa = getBeta(90, 7);
		double aaaaa = getBeta(50, 75);
		double aaaaaa = getBeta(30, 15);
		*/

		// TODO: These should be cached and updated in updateTree?
		const double uct = static_cast<double>(child.uct.wins) / static_cast<double>(child.uct.visits);
		//const double uct = child.uct.winrate;

		if (child.amaf.visits)
		{
			// TODO: These should be cached and updated in updateTree?
			const double amaf = static_cast<double>(child.amaf.wins) / static_cast<double>(child.amaf.visits);
			//const double amaf = child.amaf.winrate;

			const double beta = getBeta(child.amaf.visits, child.uct.visits);

			val = (beta * amaf) + ((1.0 - beta) * uct);
		}
		else
			val = uct;
		
		return val;
	}

	TreeNode & chooseChild(const AmafMap & moves, TreeNode & node, int& bestIdx)
	{
		int idx = 0;
		double best = std::numeric_limits<double>::min();

		node.foreachChild([&](const TreeNode& c)
		{
			double val;

			// Give preference to unvisited nodes randomly
			if (c.uct.visits == 0)
				val = 10000.0 + static_cast<double>(Random::fastRandom(10000));
			else
				val = uctRave(c) + UCT_EXPLORE 
					* std::sqrt(std::log(node.uct.visits) / c.uct.visits);

				/* // Old straight UCT
				val = (static_cast<double>(c.uct.wins)
					/ static_cast<double>(c.uct.visits))
				+ UCT_EXPLORE * std::sqrt(std::log(node.uct.visits) / c.uct.visits);
				//*/

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

