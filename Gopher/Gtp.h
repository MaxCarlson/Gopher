#pragma once
#include "Board.h"
#include <vector>

struct MoveStack
{
	std::vector<Move> moves;

	const Move& operator[](int idx) const
	{
		return moves[idx];
	}

	template<class... Args>
	decltype(auto) emplace_back(Args&& ...args)
	{
		return moves.emplace_back(std::forward<Args>(args)...);
	}

	size_t size() const
	{
		return moves.size();
	}
};

namespace Gtp
{
	void mainLoop();
}
