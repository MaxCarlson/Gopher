


#include "MonteCarlo.h"
#include "Board.h"



int main()
{
	Board board;
	board.init();

	MonteCarlo monte(board);

	Stone color = Stone::BLACK;

	for (int i = 0; i < 20000; ++i)
	{
		monte.genMove(color);
		
		color = flipColor(color);
	}

	return 0;
}