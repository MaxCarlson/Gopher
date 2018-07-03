#pragma once
#include <vector>

class GameState;

struct NetResult
{
	std::vector<std::vector<float>> output;

	inline std::vector<float>& moves() { return output[0]; }
};

namespace Net
{
	void init();
	
	// We'll eventually pass this the board
	NetResult run(const GameState& state, int color);

}

