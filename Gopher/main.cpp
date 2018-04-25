


#include "MonteCarlo.h"
#include "Board.h"



int main()
{
	Board board;

	MonteCarlo monte(board);

	printBoard(board);

	return 0;
}