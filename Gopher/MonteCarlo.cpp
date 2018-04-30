#include "MonteCarlo.h"
#include "Board.h"


struct MoveStats
{
	int games = 0;
	int wins = 0;
};

constexpr double HopelessRatio = 25.0;
constexpr int MaxGameLen = 600;
constexpr int GameSearchCount = 3000;

int MonteCarlo::playRandomGame(Board& board, int color, int length, double deathRatio)
{
	const int ourColor = color;
	color = flipColor(color);
	int pass = isPass(board.lastMove);

	while (pass < 2 && --length > 0)
	{
		coord idx = board.playRandom(static_cast<Stone>(color)); // TODO: This sometimes fails, It should not fail here (or should pass)

		if (isPass(idx))
			++pass;
		else
			pass = 0;

		if (deathRatio && std::abs(board.captures[Stone::BLACK] - board.captures[Stone::WHITE]) > deathRatio) // TODO: Take komi into account?
			break;

		color = flipColor(color);

		//board.printBoard();
	}

	const double score = board.scoreFast();
	const int result = ourColor == Stone::WHITE ? score * 2 : -(score * 2);


	//std::cout << "Random playout result for color " << printStone(ourColor) << " " << score << '\n';

	return result;
}

coord MonteCarlo::genMove(int color)
{
	int losses = 0;
	int superKo = 0;
	MoveStats moves[BoardMaxIdx] = { 0 };

	for (int i = 0; i < GameSearchCount; ++i)
	{
		Board boardCopy = board;

		const Move m = { boardCopy.playRandom(static_cast<Stone>(color)), color };

		// Don't do multi stone suicides!

		groupId& gg = boardCopy.groupAt(m.idx);
		if (!isPass(m) && !boardCopy.groupAt(m.idx))
			continue;
		
		const int result = playRandomGame(boardCopy, color, MaxGameLen, HopelessRatio);

		if (result == 0)
		{
			if (superKo > 2)
				break;
			--i;
			++superKo;
		}

		// Using 0 as pass index
		const int pos = isPass(m) ? 0 : m.idx; 

		++moves[pos].games;

		losses += result > 0;
		moves[pos].wins += 1 - (result > 0);
	}

	coord bestIdx = 0;
	double bestRatio = -1000.0;
	board.foreachPoint([&](int idx, int type)
	{
		// TODO: Play with this number. 
		// Don't want to take scores from very small sample sizes
		if (moves[idx].games < 4)
			return;

		else if (board.moveCount < 3 
			  && getX(idx) > BoardSize - 2 || getX(idx) < 3
			  || getY(idx) > BoardSize - 2 || getY(idx) < 3)
			return;

		const double ratio = static_cast<double>(moves[idx].wins) 
					       / static_cast<double>(moves[idx].games);

		if (ratio > bestRatio)
		{
			bestRatio = ratio;
			bestIdx = idx == 0 ? Pass : idx; // We use 0 as the pass idx
		}
	});

	return bestIdx;
}
