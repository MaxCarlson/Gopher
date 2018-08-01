#pragma once

struct Options
{
	int maxPlayouts			= 361;

	double resignThresh		= 0.25;
	bool validation			= false;
	int rngMovesNumber		= 30;
	int valMaxMoves			= 200;
};

inline Options options;
