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
	// TODO: Add amaf stats
}

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

void SearchTree::init(const Board& board, int color) 
{
	rootColor = color;
	expandNode(board, root, color);
}

void SearchTree::afterSearch()
{
	timer(true);
	pruneTree(root);
	std::cerr << "Tree pruned in ";
	timer(false);
	std::cerr << "s \n";

	writeOverBranch(root);
}

SearchStatistics SearchTree::getStatistics(const TreeNode& root, bool resignOnWinChance) const
{
	// The most searched move should be the best  
	// move as UCT should be searching it 
	// exponentially more than the worst

	int bestIdx = Pass;
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

		if (s.winRate < ResignThreshold && resignOnWinChance)
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
	return getStatistics(root, true).idx;
}

void SearchTree::printBestLine() const
{
	auto* node = &root;

	bool first = true;
	while (!node->isLeaf())
	{
		auto stats = getStatistics(*node, false);

		if (!stats.best)
			break;

		if (first)
		{
			std::cerr << "seq: " << moveToString(stats.idx) << " ";
			if (stats.best->uct.visits < 20)
				std::cerr << std::setw(5) << "< data " << std::setw(2) << " ";
			else
				std::cerr << std::setw(5) << std::fixed << std::setprecision(2) << stats.winRate * 100.0 << "%" << std::setw(3) << " ";
		}
		else
		{
			if (stats.best->uct.visits < 5  && stats.best->amaf.visits < 20)
				std::cerr << std::setw(5) << "< data " << std::setw(2) << " ";
			else
				std::cerr << std::setw(5) << moveToString(stats.idx) << std::setw(5) << " ";
		}

		node = stats.best;
		first = false;
	}

	std::cerr << '\n';
}

void SearchTree::allocateChildren(TreeNode & node)
{
	node.children = new UctTreeNodes;
}

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

int SearchTree::pruneTree(TreeNode& root)
{
	static unsigned long long i = 0;
	++i;

	if (i == 2885743)
		int a = 5;

	if (root.isLeaf())
		return false;

	int toDelete = 0;
	auto& children = root.children->nodes;

	if (children.size() > root.size)
		toDelete += children.size() - root.size;

	root.children->foreachChild(root.size, [&](TreeNode& child)
	{
		if (pruneTree(child))		
			++toDelete;
	});

	if (root.size == 0)
	{
		delete root.children;
		root.children = nullptr;
	}
	else
	{
		children.erase(children.end() - toDelete, children.end());
		root.size = children.size();
	}

	return !root.size;
}


/*
int SearchTree::pruneTree(TreeNode& root)
{
	if (root.isLeaf())
		return false;

	auto& children = root.children->nodes;
	static int i = 0;

	int idx = 0;
	root.children->foreachChild(root.size, [&](TreeNode& child)
	{
		if (pruneTree(child))
		{
			++i;
			if (i == 9)
				int a = 5;

			children.erase(children.begin() + idx); // TODO: Change to fast_erase with small vec
			--root.size;
		}
		++idx;
	});

	if (root.size < 1)
	{
		delete root.children;
		root.children = nullptr;
		return true;
	}
	else if (root.size < children.size() - 5 // TODO: Reexamine this
		 || (root.size < 5 && root.size < children.size()))
	{
		children.erase(children.begin() + root.size, children.end());
		children.shrink_to_fit();
		root.size = children.size();
	}

	// Root of this node didn't lose a direct child
	return false;
}

void SearchTree::pruneTree(TreeNode& root)
{
	if (root.isLeaf())
		return;

	auto& children = root.children->nodes;

	root.children->foreachChild(root.size, [&](TreeNode& child)
	{
		pruneTree(child);
	});

	// Prune all the spots for children that will no longer
	// be filled

	static int i = 0;
	if (root.size < children.size() - 1) // - 5 or - 10 ?
	{
		if (root.size)
		{
			++i;
			if (i == 101)
				int a = 5;

			children.erase(children.begin() + root.size, children.end());
			children.shrink_to_fit();
			root.size = children.size();
		}
		else
		{
			delete root.children;
			root.children = nullptr;
			root.size = 0;
		}
	}
}
*/

void SearchTree::recordSearchResults(const AmafMap& moves, int color, bool isWin)
{
	RAVE::updateTree(moves, &root, rootColor, color, isWin);
}

