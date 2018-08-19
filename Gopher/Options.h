#pragma once
#include <string>

// Struct containing options that can be changed at runtime with command line
// and eventually through a dialog option after engine start
struct Options
{
	int maxPlayouts			= 361;

	double resignThresh		= 0.25;
	bool validation			= false;
	int rngMovesNumber		= 30;
	int valMaxMoves			= 200;

	// Model options
	std::string path		= "models/GoNet.dnn";
	int netHistory			= 3; // Number of previous board states that are fed to the model TODO: (this would be nice to auto detect!)
	// These are calculated from netHistory prior to running the network.
	int boardDepth; 
	int inputSize;		
};

inline Options options;