#include "MonteCarlo.h"
#include "Board.h"


struct MoveStats
{
	int wins = 0;
	int losses = 0;
};

constexpr double SearchTime = 5.0;
constexpr int MaxGameLen = 200;
constexpr int GameSearchCount = 10000;

int MonteCarlo::playRandomGame(int length, double deathRatio)
{
	return 0;
}

coord MonteCarlo::genMove(int color)
{
	MoveStats moves[BoardRealSize] = { 0 };

	for (int i = 0; i < GameSearchCount; ++i)
	{
		Board boardCopy = board;

		Move m = { boardCopy.playRandom(static_cast<Stone>(color)), color };

		// Don't do multi stone suicides!
		if (!isPass(m) && !boardCopy.groupAt(m.idx))
			continue;
		
		playRandomGame(MaxGameLen, 0.1);
	}
}
