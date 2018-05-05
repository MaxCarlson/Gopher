#include "SearchTree.h"
#include "Board.h"


size_t UctNodeBase::size() const
{
	return children ? children->nodes.size() : 0;
}

bool UctNodeBase::isLeaf() const
{
	return !children;
}

void SearchTree::init(const Board& board, int color) 
{
	baseColor = color;
	expandNode(board, root, color);
	++root.visits;
}

coord SearchTree::getBestMove() const
{
	int idx = 0;
	int bestIdx = 0;
	int bestVisits = 0;
	//double bestWinRate = std::numeric_limits<double>::min();
	for (const auto& it : root.children->nodes)
	{
		/*
		double wr = static_cast<double>(it.wins) / static_cast<double>(it.visits);
		if (wr > bestWinRate)
		{
			bestWinRate = wr;
			bestIdx = idx;
		}
		*/

		// This should be the best move as UCT 
		// should be searching it exponentially more
		// than the worst
		if (it.visits > bestVisits)
		{
			bestVisits = it.visits;
			bestIdx = idx;
		}

		++idx;
	}

	return root.children->nodes[bestIdx].idx;
}

void SearchTree::allocateChildren(UctNodeBase & node)
{
	node.children = new UctTreeNodes;
}

void deallocateNode(UctNodeBase& root)
{
	if (!root.children)
		return;
	
	for (auto& n : root.children->nodes)
		deallocateNode(n);
	
	delete root.children;
}

inline void writeOverBranch(UctNodeBase& root)
{
	root.visits = root.wins = root.idx = 0;
	
	if(root.children)
		for (auto& n : root.children->nodes)
			writeOverBranch(n);
}

// It seems prudent not to write over the branch we end 
// up choosing (and the oponents reply) 
// TODO: 
// Integrate this idea
void SearchTree::writeOverTree()
{
	for (auto& n : root.children->nodes)
		writeOverBranch(root);
}

void SearchTree::expandNode(const Board & board, UctNodeBase & node, int color)
{
	if (!board.free.size())
		return;

	if (!node.children)
		allocateChildren(node);


	board.foreachFreePoint([&](coord idx)
	{
		if (!board.isValidNoSuicide({ idx, color }))
			return;

		node.children->nodes.emplace_back(idx); // Likely optimization point
		// TODO: Possibly count children, resize vector to that then add them?
	});
}

void SearchTree::recordSearchResults(SmallVec<int, 100>& moves, int color, bool isWin)
{
	if (!isWin)
		return;

	UctNodeBase* node = &root;
	node->wins += isWin;

	// Loop through all nodes we moved through 
	// and score them relative to side to move
	int depth = 0;
	for (const auto it : moves)
	{
		node = &node->children->nodes[it];
		node->wins += isWin;

		std::cout << "NodeInfo: Depth-" << depth++ << " Visits-" << node->visits << " Wins-" << node->wins << '\n';
	}
	std::cout << '\n';
}

