#pragma once
#include <cstdint>

static unsigned long RSeed = 29264;

uint16_t fastRandom(unsigned int max)
{
	RSeed = ((RSeed * 1103515245) + 12345) & 0x7fffffff;
	return ((RSeed & 0xffff) * max) >> 16;
}