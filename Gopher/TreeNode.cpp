#include "TreeNode.h"

TreeNode::~TreeNode()
{
	if (children)
	{
		delete children;
		children = nullptr;
	}
}

bool TreeNode::isLeaf() const
{
	return !children;
}

void TreeNode::clearStats()
{
	uct.clear();
	amaf.clear();
}