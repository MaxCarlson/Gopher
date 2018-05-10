#include "SearchTree.h"
#include "Board.h"
#include "Threads.h"
#include "Amaf.h"

#include <iomanip>

inline void printNode(const UctNodeBase& it, int color)
{
	std::cout << moveToString({ it.idx, color })
		<< " Visits: "   << std::setw(5) << it.visits << " | "
		<< " Wins: "     << std::setw(5) << it.wins   << " | "
		<< " Children: " << std::setw(3) << it.size   << " | "
		<< " WinRate: "  << static_cast<double>(it.wins) / static_cast<double>(it.visits) << '\n';
}

bool UctNodeBase::isLeaf() const
{
	return !children;
}

void SearchTree::init(const Board& board, int color) 
{
	rootColor = color;
	expandNode(board, root, color);
	++root.visits;
}

void SearchTree::afterSearch()
{
	writeOverBranch(root);
}

SearchStatistics SearchTree::getStatistics() const
{
	int idx = 0;
	coord bestIdx = 0;
	int bestVisits = 0;

	// The most searched move should be the best  
	// move as UCT should be searching it 
	// exponentially more than the worst
	for (const auto& it : root.children->nodes)
	{
		//printNode(it, baseColor);

		if (it.visits > bestVisits)
		{
			bestVisits = it.visits;
			bestIdx = idx;
		}
		++idx;
	}

	// Resign when we're winning less than this many games in play
	// TODO: Make this dynamic?
	static constexpr double ResignThreshold = 0.000005;

	const auto& bestNode = root.children->nodes[bestIdx];
	bestIdx = bestNode.idx;

	double winRate = static_cast<double>(bestNode.wins)
				   / static_cast<double>(bestNode.visits);

	if (winRate < ResignThreshold)
		bestIdx = Resign;

	return { bestIdx, winRate };
}

coord SearchTree::getBestMove() const
{
	return getStatistics().bestIdx;
}

void SearchTree::printStatistics() const
{
	const auto stats = getStatistics();
	Move move = { stats.bestIdx, rootColor };
	auto xy = moveToXY(move);

	std::cerr << moveToString(move) << " with " << stats.winRate << " winrate \n";
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

void SearchTree::recordSearchResults(const AmafMap& moves, int color, bool isWin)
{
	if (color != rootColor)
		isWin = !isWin;

	UctNodeBase* node = &root;

	node->wins += isWin;

	// Starting from root
	// loop through all nodes we moved through 
	// and score them relative to side to move
	//int depth = 1;
	for(int i = 0; i < moves.root; ++i)
	{
		const auto it = moves.moves[i];

		node = &node->children->nodes[it];
		node->wins += isWin;
		isWin = !isWin;
		//++depth;
	}

	//std::cout << "Max Depth Reached " << depth << '\n';

}

