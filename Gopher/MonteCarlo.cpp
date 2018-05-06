#include "MonteCarlo.h"
#include "Board.h"
#include "MovePicker.h"
#include "Playout.h"

struct MoveStats
{
	int games = 0;
	int wins = 0;
};

constexpr double HopelessRatio = 25.0;
constexpr int MaxGameLen = 400; 
constexpr int GameSearchCount = 55000 * (9.0 / BoardSize) / 2;
// Resign when we're only winning this *100 % of games
constexpr double ResignRatio = 0.1;


// TODO: If oponnent passes and we're winning by official score, pass as well!

// Tree-less Monte carlo search. Very weak without heuristics
coord MonteCarlo::genMove(int color)
{
	int losses = 0;
	int superKo = 0;
	MoveStats moves[BoardMaxIdx] = { 0 };

	//timer(true);

	int goodGames = 0;
	for (int i = 0; i < GameSearchCount; ++i)
	{
		Board boardCopy = board;

		//const Move m = { boardCopy.playRandom(static_cast<Stone>(color)), color };
		const Move m = MovePicker::pickMove(boardCopy, static_cast<Stone>(color));

		// Don't do multi stone suicides!
		groupId& gg = boardCopy.groupAt(m.idx);
		if (!isPass(m) && !boardCopy.groupAt(m.idx))
			continue;
		
		const int result = Playouts::playRandomGame(boardCopy, color, MaxGameLen, HopelessRatio);

		if (result == 0)
		{	// Superko, ignore this playout and try again
			--i;
			++superKo;
			continue;
		}

		// Using 0 as pass index
		const int pos = isPass(m) ? 0 : m.idx; 

		++moves[pos].games;
		++goodGames;

		losses += result > 0;
		moves[pos].wins += (result > 0);

		// TODO: Winning a lot early quit
	}

	// Resign if ratio of matches we win is below ResignRatio
	// TODO: Doesn't seem to be working?
	coord bestIdx = Resign;
	double bestRatio = ResignRatio;

	if (!goodGames)
	{
		bestIdx = Pass;
	}
	else
	{
		board.foreachPoint([&](int idx, int type)
		{
			// TODO: Play with this number. 
			// Don't want to take scores from very small sample sizes
			if (moves[idx].games < 4)
				return;

			else if (board.moveCount < 3
				&& (getX(idx) > BoardSize - 2 || getX(idx) < 3
				|| getY(idx) > BoardSize - 2 || getY(idx) < 3))
				return;

			const double ratio = static_cast<double>(moves[idx].wins)
				/ static_cast<double>(moves[idx].games);

			if (ratio > bestRatio)
			{
				bestRatio = ratio;
				bestIdx = idx == 0 ? Pass : idx; // We use 0 as the pass idx
			}
		});
	}

	//timer(false);

	return bestIdx;
}
