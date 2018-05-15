#include "TreeNode.h"

TreeNode::~TreeNode()
{
	/* // Can't use this as if it gets called from a vector,
	   // We'll delete any children we have on resize
	if (children)
	{
		delete children;
		children = nullptr;
	}
	*/
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

void TreeNode::allocateChildren()
{
	children = new UctTreeNodes;
}

void TreeNode::addChild(coord pos)
{
	if (size >= children->nodes.size())
		children->nodes.emplace_back(pos);
	else
		children->nodes[size].idx = pos;
	++size;
}

void deallocChildren(TreeNode& root)
{
	if (root.children)
	{
		for (auto& c : root.children->nodes)
			deallocChildren(c);

		root.children->nodes.clear();

		delete root.children;
		root.children = nullptr;
	}
	root.size = 0;
}

void TreeNode::deallocateChildren()
{
	deallocChildren(*this);
}

/*
void TreeNode::deallocateRange(int first, int end)
{
	for (int i = first; i < end; ++i)
		deallocChildren(children->nodes[i]);

	size -= end - first;
}
*/
