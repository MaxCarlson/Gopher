#pragma once
#include <vector>

struct UctNode
{
	int wins		= 0;
	int visits		= 0;
	bool expanded	= false;

	void expand();
	void selectChild(int color);
	bool isExpanded();

	std::vector<UctNode> children;
};

