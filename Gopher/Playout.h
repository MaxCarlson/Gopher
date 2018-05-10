#pragma once

struct Board;
struct AmafMap;

namespace Playouts
{
	int playRandomGame(Board& board, AmafMap& amaf, int color, int length, double deathRatio);
}
