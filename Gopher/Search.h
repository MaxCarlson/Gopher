#pragma once

struct Board;
using coord = int;


class Search
{
public:
	coord search(const Board& board, int color);
};

