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

	return true;

	// Is this an optimization? Why not allow non-eyelike - non ko
	// moves if there's a group of stones ready to be captured on the board?
	/*
	foreach_neighbor(board, coord, {
		group_t g = group_at(board, c);
		groups_in_atari += (board_group_info(board, g).libs == 1);
	});
	return !!groups_in_atari;
	*/
}

bool Board::tryRandomMove(Stone color, coord& idx, int rng) const 
{
	idx = free[rng];

	Move m = { idx, color };

	return true;
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

