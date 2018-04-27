


#include "MonteCarlo.h"
#include "Board.h"



int main()
{
	Board board;
	board.init();

	MonteCarlo monte(board);

	for (int i = 0; i < 20000; ++i)
	{
		Stone c = i % 2 == 0 ? Stone::BLACK : Stone::WHITE;
		printBoard(board);
		board.playRandom(c);
	}

	return 0;
}