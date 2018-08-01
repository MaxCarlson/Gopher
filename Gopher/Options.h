#pragma once

struct Options
{
	double resignThresh		= 0.1;
	bool validation			= false;
	int rngMovesNumber		= 30;
	int valMaxMoves			= 200;
};

inline Options options;
