#include "SearchTree.h"
#include "Board.h"
#include "Threads.h"
#include "Amaf.h"

#include <iomanip>

inline void printNode(const TreeNode& it, int color)
{
	std::cout << moveToString({ it.idx, color })
		<< " Visits: "   << std::setw(5) << it.uct.visits << " | "
		<< " Wins: "     << std::setw(5) << it.uct.wins   << " | "
		<< " Children: " << std::setw(3) << it.size   << " | "
		<< " WinRate: "  << static_cast<double>(it.uct.wins) / static_cast<double>(it.uct.visits) << '\n';
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

void SearchTree::init(const Board& board, int color) 
{
	rootColor = color;
	expandNode(board, root, color);
}

void SearchTree::afterSearch()
{
	writeOverBranch(root);
}

SearchStatistics SearchTree::getStatistics() const
{
	coord bestIdx = Pass;
	int bestVisits = 0;

	// The most searched move should be the best  
	// move as UCT should be searching it 
	// exponentially more than the worst

	for (int idx = 0; idx < root.size; ++idx)
	{
		auto& node = root.children->nodes[idx];
		if (node.uct.visits > bestVisits)
		{
			bestVisits = node.uct.visits;
			bestIdx = idx;
		}
	}

	// Resign when we're winning less than this many games in play
	// TODO: Make this dynamic?
	static constexpr double ResignThreshold = 0.000005;

	const auto& bestNode = root.children->nodes[bestIdx];
	bestIdx = bestNode.idx;

	double winRate = static_cast<double>(bestNode.uct.wins)
				   / static_cast<double>(bestNode.uct.visits);

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

void SearchTree::allocateChildren(TreeNode & node)
{
	node.children = new UctTreeNodes;
}

void deallocateNode(TreeNode& root)
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
void SearchTree::writeOverBranch(TreeNode& root)
{
	root.clearStats();
	
	if(root.children)
		root.children->foreachChild(root.size, [&](TreeNode& n)
		{
			writeOverBranch(n);
		});
	root.size = 0;
}

void SearchTree::expandNode(const Board & board, TreeNode & node, int color)
{
	if (!board.free.size())
		return;

	if (!node.children)
		allocateChildren(node);

	int i = 0;
	board.foreachFreePoint([&](coord idx) // Likely optimization point
	{
		if (!board.isValidNoSuicide({ idx, !color }))
			return;

		if (node.size >= node.children->nodes.size())
			node.children->nodes.emplace_back(idx); 
		else
			node.children->nodes[i].idx = std::move(idx); // TODO: Customize a SmallVec like data structure
		++i;
		++node.size;
	});
}

void SearchTree::recordSearchResults(const AmafMap& moves, int color, bool isWin)
{
	RAVE::updateTree(moves, &root, rootColor, color, isWin);
}

