#include "Gtp.h"
#include "Random.h"
#include "defines.h"
#include <chrono>

int main()
{
	// Quick and dirty fix to make random number generator non-deterministic
	// for purposes of self-play
	std::random_device rd;
	std::uniform_int_distribution<int> dist(0, 9999);
	for (int i = 0; i < dist(rd); ++i)
		Random::fastRandom(10000);

	Gtp::mainLoop();

	return 0;
}