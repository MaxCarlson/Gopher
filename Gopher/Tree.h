#pragma once

struct UctNode;
class GameState;
struct Board;
using coord = int;

namespace Tree
{
	UctNode& getRoot();
	void initRoot(const Board & board, GameState& state, int color);
	UctNode& findBestMove();
	void switchRoot(UctNode& best);
}

