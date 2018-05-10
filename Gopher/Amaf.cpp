#include "Amaf.h"
#include <array>


namespace RAVE
{
	void updateTree(const AmafMap & moves, TreeNode * root, int toMove, int color, bool isWin)
	{
		if (toMove != color)
			isWin = !isWin;

		// Build a map of the moves made during playout
		// so we can quickly look them up as well as 
		// the order in which they were played
		static constexpr int EMPTY = std::numeric_limits<int>::max();

		std::array<int, BoardRealSize2> movesByIdx;
		movesByIdx.fill(EMPTY);

		for (int i = 0; i < moves.moves.size(); )
			movesByIdx[moves.moves[i]] = ++i;


	}
}

