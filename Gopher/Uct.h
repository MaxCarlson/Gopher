#pragma once


class Board;
using coord = int;
struct UctNodeBase;

class Uct
{
public:

	coord search(const Board& board, int color);

private:

	// Find the best child of the node based on UCT's algo
	UctNodeBase& chooseChild(const UctNodeBase& node) const;

};

inline Uct uct;

