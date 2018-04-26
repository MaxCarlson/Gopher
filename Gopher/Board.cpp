#include "Board.h"
#include "Random.h"
#include <iostream>

inline int getIdx(int x, int y)
{
	return y * BoardRealSize + x;
}

void printPos(const int i)
{
	switch (i)
	{
	case Stone::NONE:
		std::cout << "   ";
		break;
	case Stone::BLACK:
		std::cout << " b ";
		break;
	case Stone::WHITE:
		std::cout << " w ";
		break;
	case Stone::OFFBOARD:
		std::cout << " X ";
		break;
	default:
		std::cout << "BoardIdxIssue!";
	}
}

void printRow(const Board& board, int row)
{
	std::cout << "| ";
	for (int i = 0; i < BoardRealSize; ++i)
		printPos(board.points[getIdx(i, row)]);
	std::cout << " | \n";
}

void printBoard(const Board & board)
{
	std::cout << '\n';
	for (int i = 0; i < BoardRealSize; ++i)
	{
		printRow(board, i);
	}
	std::cout << '\n';
}

void Board::init()
{
	std::memset(this, 0, sizeof(Board));

	// Set offboard margin
	int topRow = BoardRealSize2 - BoardRealSize;
	for (int i = 0; i < BoardRealSize; ++i)
		points[i] = points[i + topRow] = Stone::OFFBOARD;

	for (int i = 0; i <= topRow; i += BoardRealSize)
		points[i] = points[BoardRealSize - 1 + i] = Stone::OFFBOARD;

	// Set neighbor values for all points that aren't offboard
	// And create list of free positions
	foreachPoint([&](coord idx)
	{
		if (points[idx] == Stone::OFFBOARD)
			return;
		neighbors[idx].increment(static_cast<Stone>(points[idx]));

		free.emplace_back(idx);
	});

	// TODO: Generate zobrist hash stuff
}

int Board::neighborCount(coord idx, Stone color) const
{
	return neighbors[idx].n[color];
}

bool Board::isEyeLike(coord idx, Stone color) const
{
	return (neighborCount(idx, color) + neighborCount(idx, Stone::OFFBOARD)) == 4;
}

bool Board::isFalseEyelike(coord idx, Stone color) const
{
	int numNeighbors[Stone::MAX] = { 0, 0, 0, 0 };

	eachDiagonalNeighbor(idx, [](auto n)
	{
		++numNeighbors(at(idx));
	});

	// Do we have two enemy stones diagonally (or side of the board
	// equivalent) ?
	numNeighbors[flipColor(color)] += !!numNeighbors[Stone::OFFBOARD];
	return numNeighbors[flipColor(color)] >= 2;
}

bool Board::isOnePointEye(coord idx, Stone color) const
{
	return isEyeLike(idx, color) && !isFalseEyelike(idx, color);
}

bool Board::isValid(const Move & m) const
{
	if(points[m.idx] != Stone::NONE)
		return false;

	if (!isEyeLike(m.idx, flipColor(m.color)))
		return true;

	// No ko's
	if (ko == m)
		return false;

	return true;

	// Is this an optimization? Why not allow non-eyelike - non ko
	// moves if there's a group of points ready to be captured on the board (Atari)?
	/*
	foreach_neighbor(board, coord, {
		group_t g = group_at(board, c);
		groups_in_atari += (board_group_info(board, g).libs == 1);
	});
	return !!groups_in_atari;
	*/
}

bool Board::tryRandomMove(Stone color, coord& idx, int rng)  // TODO: Pass a playout policy instead of just looking at move validity
{
	idx = free[rng];

	Move m = { idx, color };

	if (isOnePointEye(m.idx, m.color) || !isValid(m))
		return false;

	return true;
}

coord Board::playRandom(Stone color) 
{
	coord idx;
	if (!free.size())
	{
		// Handle no moves left
	}

	int rng = fastRandom(free.size());
	for (int i = rng; i < free.size(); ++i)
		if (tryRandomMove(color, idx, rng))
			return idx;
	
	for(int i = 0; i < rng; ++i)
		if (tryRandomMove(color, idx, rng))
			return idx;
}

void Board::makeMove(const Move & m)
{
}
