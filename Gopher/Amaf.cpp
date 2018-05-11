#include "Amaf.h"
#include <array>
#include "SearchTree.h"


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
}

