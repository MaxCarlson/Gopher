#include "MonteCarlo.h"
#include "Board.h"
#include "Gtp.h"



int main()
{
	/*
	Board board;
	board.init();

	MonteCarlo monte(board);

	Stone color = Stone::BLACK;

	for (int i = 0; i < 20000; ++i)
	{
		coord idx = monte.genMove(color);

		board.makeMove({ idx, color });
		board.printBoard();
		
		color = flipColor(color);
	}
	*/

	mainLoop();

	return 0;
}