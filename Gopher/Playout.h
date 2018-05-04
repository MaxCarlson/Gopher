#pragma once

struct Board;

namespace Playouts
{
	int playRandomGame(Board& board, int color, int length, double deathRatio);
}
