#include "Tree.h"

void UctNode::expand()
{
	expanded = true;
}

void UctNode::selectChild(int color)
{

}

bool UctNode::isExpanded()
{
	return expanded;
}
