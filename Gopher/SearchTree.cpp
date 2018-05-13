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

SearchStatistics SearchTree::getStatistics(const TreeNode& root) const
{
	// The most searched move should be the best  
	// move as UCT should be searching it 
	// exponentially more than the worst

	int bestIdx = 0;
	int bestVisits = 0;
	for (int idx = 0; idx < root.size; ++idx)
	{
		auto& node = root.children->nodes[idx];
		if (node.uct.visits > bestVisits)
		{
			bestVisits = node.uct.visits;
			bestIdx = idx;
		}
	}

	SearchStatistics s;

	if (bestIdx != Pass)
	{
		s.best = &root.children->nodes[bestIdx];

		static constexpr double ResignThreshold = 0.0005;

		s.winRate = static_cast<double>(s.best->uct.wins)
				  / static_cast<double>(s.best->uct.visits);

		if (s.winRate < ResignThreshold)
			s.idx = Resign;
		else
			s.idx = s.best->idx;
	}
	else
	{
		s.idx = Pass;
		s.winRate = 0.0; // TODO: This is misleading
		s.best = nullptr;
	}

	return s;
}

coord SearchTree::getBestMove() const
{
	return getStatistics(root).idx;
}

void SearchTree::printBestLine() const
{
	auto* node = &root;

	bool first = true;
	while (!node->isLeaf())
	{
		auto stats = getStatistics(*node);

		if (!stats.best)
			break;

		if (first)
			std::cerr << "seq: " << moveToString(stats.idx) << " " << std::setw(4) << std::fixed << std::setprecision(2) << stats.winRate * 100.0 << "% ";
		else
			std::cerr << moveToString(stats.idx) << " ";

		node = stats.best;
		first = false;
	}

	std::cerr << '\n';
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

