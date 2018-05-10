#include "Playout.h"
#include "Board.h"
#include "MovePicker.h"
#include "Amaf.h"

namespace Playouts
{
	int playRandomGame(Board& board, AmafMap& amaf, int color, int length, double deathRatio)
	{
		const int ourColor = color;
		color = flipColor(color);
		int pass = isPass(board.lastMove);

		while (pass < 2 && --length > 0)
		{
			const Move m = MovePicker::pickMove(board, color);

			amaf.addMove(m.idx);

			if (isPass(m.idx))
				++pass;
			else
				pass = 0;

			if (deathRatio && std::abs(board.captures[Stone::BLACK] - board.captures[Stone::WHITE]) > deathRatio) 
				break;

			color = flipColor(color);
		}

		const double score = board.scoreFast();
		const int result = ourColor == Stone::WHITE ? score * 2 : -(score * 2);

		//std::cout << "Random playout result for color " << printStone(ourColor) << " " << score << '\n';

		return result;
	}
}

