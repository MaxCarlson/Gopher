#pragma once

struct UctNode;
using coord = int;

namespace Tree
{
	UctNode& getRoot();
	coord findBestMove();
}

