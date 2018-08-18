#pragma once

// Struct containing options that can be changed at runtime with command line
// and eventually through a dialog option after engine start
struct Options
{
	int maxPlayouts			= 361;

	double resignThresh		= 0.25;
	bool validation			= false;
	int rngMovesNumber		= 30;
	int valMaxMoves			= 200;
};

inline Options options;
