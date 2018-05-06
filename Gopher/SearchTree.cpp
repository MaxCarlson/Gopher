#include "SearchTree.h"
#include "Board.h"


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

void SearchTree::afterSearch()
{
	writeOverBranch(root);
}

coord SearchTree::getBestMove() const
{
	int idx = 0;
	coord bestIdx = 0;
	int bestVisits = 0;

	// The most searched move should be the best  
	// move as UCT should be searching it 
	// exponentially more than the worst
	for (const auto& it : root.children->nodes)
	{
		if (it.visits > bestVisits)
		{
			bestVisits = it.visits;
			bestIdx = idx;	
		}
		++idx;
	}

	static constexpr double ResignThreshold = 0.05;

	const auto& bestNode = root.children->nodes[bestIdx];
	bestIdx = bestNode.idx;

	double winRate = static_cast<double>(bestNode.wins)
				   / static_cast<double>(bestNode.visits);

	// Resign if the best move candidates winrate is less
	// than ResignThreshold
	if (winRate < ResignThreshold)
		bestIdx = Resign;

	return bestIdx;
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

// It seems prudent not to write over the branch we end 
// up choosing (and the oponents reply) 
// TODO: 
// Integrate this idea
//
// Actually this might slow things down, lot's of work 
// moving nodes down the tree
void SearchTree::writeOverBranch(UctNodeBase& root)
{
	root.visits = root.wins = root.idx = 0;
	
	if(root.children)
		root.children->foreachChild(root.size, [&](UctNodeBase& n)
		{
			writeOverBranch(n);
		});
	root.size = 0;
}

void SearchTree::expandNode(const Board & board, UctNodeBase & node, int color)
{
	if (!board.free.size())
		return;

	if (!node.children)
		allocateChildren(node);

	int i = 0;
	board.foreachFreePoint([&](coord idx)
	{
		if (!board.isValidNoSuicide({ idx, color }))
			return;

		if (node.size >= node.children->nodes.size())
			node.children->nodes.emplace_back(idx); // Likely optimization point
		else
			node.children->nodes[i].idx = std::move(idx); // TODO: Customize a SmallVec like data structure
		++i;
		++node.size;
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
	//std::cout << "NodeInfo: Depth-" << depth++ << " Visits-" << node->visits << " Wins-" << node->wins << '\n';

	for (const auto it : moves)
	{
		node = &node->children->nodes[it];
		node->wins += isWin;
	//	std::cout << "NodeInfo: Depth-" << depth++ << " Visits-" << node->visits << " Wins-" << node->wins << '\n';
	}

	//std::cout << '\n';
}

