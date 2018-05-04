#include "Playout.h"
#include "Board.h"
#include "MovePicker.h"

namespace Playouts
{
	int playRandomGame(Board& board, int color, int length, double deathRatio)
	{
		const int ourColor = color;
		color = flipColor(color);
		int pass = isPass(board.lastMove);

		while (pass < 2 && --length > 0)
		{
			//coord idx = board.playRandom(static_cast<Stone>(color)); // TODO: This sometimes fails, It should not fail here (or should pass)
			const Move m = MovePicker::pickMove(board, color);

			if (isPass(m.idx))
				++pass;
			else
				pass = 0;

			if (deathRatio && std::abs(board.captures[Stone::BLACK] - board.captures[Stone::WHITE]) > deathRatio) // TODO: Take komi into account?
				break;

			color = flipColor(color);

			//if(length == 196)
			//	std::cout << length << '\n';
			//board.printBoard();
		}

		const double score = board.scoreFast();
		const int result = ourColor == Stone::WHITE ? score * 2 : -(score * 2);

		//std::cout << "Random playout result for color " << printStone(ourColor) << " " << score << '\n';

		return result;
	}
}

