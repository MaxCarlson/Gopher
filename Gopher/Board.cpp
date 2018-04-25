#include "Board.h"
#include <iostream>


void printPos(const int i)
{
	switch (i)
	{
	case Stone::NONE:
		std::cout << " ";
		break;
	case Stone::BLACK:
		std::cout << "b";
		break;
	case Stone::WHITE:
		std::cout << "w";
		break;
	case Stone::OFFBOARD:
		std::cout << "X";
		break;
	default:
		std::cout << "BoardIdxIssue!";
	}
}

void printBoard(const Board & board)
{
	std::cout << '\n';
	for (int i = 0; i < BoardMaxIdx; ++i)
	{
		printPos(board.stones[i]);
		if (i % BoardSize == 0)
			std::cout << '\n';
	}
}
