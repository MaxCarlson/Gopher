#pragma once

struct UctNode;
class GameState;
struct Board;
using coord = int;

namespace Tree
{
	UctNode& getRoot();
	void initRoot(const Board & board, GameState& state, int color);
	UctNode& findBestMove(UctNode* node);
	void switchRoot(UctNode& best);
	void printStats(int color);
	void printNodeInfo(UctNode* node);
}

