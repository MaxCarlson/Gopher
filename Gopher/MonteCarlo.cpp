#include "MonteCarlo.h"
#include "Board.h"


struct MoveStats
{
	int games = 0;
	int wins = 0;
};

constexpr double SearchTime = 5.0;
constexpr int MaxGameLen = 600;
constexpr int GameSearchCount = 10000;

int MonteCarlo::playRandomGame(Board& board, int color, int length, double deathRatio)
{
	const int ourColor = color;
	int pass = isPass(board.lastMove);

	while (pass < 2 && --length > 0)
	{
		coord idx = board.playRandom(static_cast<Stone>(color));

		if (isPass(idx))
			++pass;
		else
			pass = 0;

		if (deathRatio && std::abs(board.captures[Stone::BLACK] - board.captures[Stone::WHITE]) > deathRatio)
			break;

		color = flipColor(color);
	}

	const double score = board.scoreFast();
	const int result = ourColor == Stone::WHITE ? score * 2 : -(score * 2);


	std::cout << "Random playout result for color " << printStone(ourColor) << " " << score << '\n';

	return result;
}

coord MonteCarlo::genMove(int color)
{
	int losses = 0;
	MoveStats moves[BoardRealSize] = { 0 };

	for (int i = 0; i < GameSearchCount; ++i)
	{
		Board boardCopy = board;

		const Move m = { boardCopy.playRandom(static_cast<Stone>(color)), color };

		// Don't do multi stone suicides!
		if (!isPass(m) && !boardCopy.groupAt(m.idx))
			continue;
		
		const int result = playRandomGame(boardCopy, color, MaxGameLen, 0.1);

		if (result == 0)
		{
			// Superko
			int a = 5;
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
		const double ratio = moves[idx].wins / moves[idx].games;

		if (ratio > bestRatio)
		{
			bestRatio = ratio;
			bestIdx = idx == 0 ? Pass : 0; // We use 0 as the pass idx
		}
	});
}
