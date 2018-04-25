#include "Board.h"
#include "Random.h"
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

int Board::neighborCount(coord idx, Stone color) const
{
	return neighbors[idx].n[color];
}

bool Board::isEyeLike(coord idx, Stone color) const
{
	return (neighborCount(idx, color) + neighborCount(idx, Stone::OFFBOARD)) == 4;
}

bool Board::isValid(const Move & m) const 
{
	if(stones[m.idx] != Stone::NONE)
		return false;

	if (!isEyeLike(m.idx, flipColor(m.color)))
		return true;

	// No ko's
	if (ko == m)
		return false;
}

bool Board::tryRandomMove(Stone color, coord& idx, int rng) const 
{
	idx = free[rng];

	Move m = { idx, color };

}

coord Board::playRandom(Stone color) 
{
	coord idx;
	if (!freeCount)
	{
		// Handle no moves left
	}

	int rng = fastRandom(freeCount);
	for (int i = rng; i < freeCount; ++i)
		if (tryRandomMove(color, idx, rng))
			return idx;
	
	for(int i = 0; i < rng; ++i)
		if (tryRandomMove(color, idx, rng))
			return idx;
}

