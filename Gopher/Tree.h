#pragma once

struct UctNode;
class GameState;
struct Board;
using coord = int;

// TODO: Probably make this into a class/struct
namespace Tree
{
	UctNode& getRoot();
	void initRoot(const Board & board, GameState& state, int color);
	UctNode& findBestMove(UctNode* node);
	void updateRoot(const Board & board, GameState& state, int color, int bestIdx);
	void updateRoot(UctNode& best);
	void printStats(int color);

	// Debug function for looking at nodes and their
	// network evaluations
	void printNodeInfo(UctNode* node);
}

