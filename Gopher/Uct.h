#pragma once


class Board;
using coord = int;

class Uct
{
public:
	
	coord search(const Board& board, int color);
};

inline Uct uct;

