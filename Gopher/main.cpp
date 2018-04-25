


#include "MonteCarlo.h"
#include "Board.h"



int main()
{
	Board board;
	board.init();

	MonteCarlo monte(board);

	printBoard(board);

	return 0;
}