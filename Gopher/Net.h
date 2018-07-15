#pragma once
#include "defines.h"
#include <vector>
#include <cmath>
#include <functional>
#include <algorithm>
class GameState;

// Apply softmax to the input vector
template<class T>
void softmax(std::vector<T>& vec)
{
	const auto maxEle = *std::max_element(std::begin(vec), std::end(vec));
	T total = 0;

	// TODO: Look into effeciancies of this method!
	std::transform(std::begin(vec),
		std::end(vec),
		std::begin(vec),
		[&](auto x)
	{
		auto ex = std::exp(x - maxEle);
		total += ex;
		return ex;
	});
	std::transform(std::begin(vec),
		std::end(vec),
		std::begin(vec),
		[&](T v)
	{
		return v / total;
	});
}

struct NetResult
{
	enum
	{
		MOVES,
		WIN_CHANCE
	};

	NetResult()
	{
		// TODO: Maybe these should be arrays if we're not doing a direct copy from CNTK?
		output.resize(2);
		output[MOVES].resize(BoardSize2);
		output[WIN_CHANCE].resize(2);
	}

	std::vector<std::vector<float>> output;

	enum Side
	{
		TO_MOVE,
		OPPONENT
	};

	std::vector<float>& moves()		 { return output[MOVES];	  }
	std::vector<float>& winChances() { return output[WIN_CHANCE]; }

	float winChance(Side side) const
	{
		return output[WIN_CHANCE][side];
	}

	void process()
	{
		softmax(output[WIN_CHANCE]);
	}
};

namespace Net
{
	void init();
	
	// We'll eventually pass this the board
	NetResult run(const GameState& state, int color);

}

