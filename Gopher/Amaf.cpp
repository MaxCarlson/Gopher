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

		// The moves from 0 - moves.root are actually the index
		// of the moves within the vector of children. Don't look at them for AMAF
		// TODO: Should this be changed?
		for (int i = moves.root; i < moves.moves.size(); )
			movesByIdx[moves.moves[i]] = ++i;

		int i = 0;
		node->wins += isWin;
		node = &node->children->nodes[moves.moves[i++]];

		while (node)
		{
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
				const int dist = firstPlayed - i + 1;
				if (dist & 1)
					return;

				// TODO: Later moves are worth less
				// TODO: Make sure amafMap during playout doesn't 
				// overwrite moves closer to the root with older moves

				++child.amaf.visits;
				child.amaf.wins += isWin;
			});

			isWin = !isWin;

			if (node->isLeaf())
				return;

			node = &node->children->nodes[moves.moves[i++]];
		}
	}
}

