#include "SearchTree.h"
#include "Board.h"
#include "Threads.h"
#include "Amaf.h"
#include "TreeNode.h"
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

void SearchTree::init(const Board& board, int color) 
{
	rootColor = color;
	expandNode(board, root, color);
}

void SearchTree::afterSearch()
{
	auto st = Time::startTimer();
	pruneTree(root);
	writeOverBranch(root);

	std::cerr << "Tree pruned in " << Time::endTime<std::chrono::duration<double>>(st) << "s \n";
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

void SearchTree::expandNode(const Board & board, TreeNode & node, int color)
{
	if (!board.free.size())
		return;

	if (!node.children)
		node.allocateChildren();

	board.foreachFreePoint([&](coord idx) 
	{
		if (!board.isValidNoSuicide({ idx, !color }))
			return;

		node.addChild(idx);
	});
}

void SearchTree::writeOverBranch(TreeNode& root)
{
	root.clearStats();
	
	if(root.children)
		root.foreachChild([&](TreeNode& child)
		{
			writeOverBranch(child);
		});
	root.size = 0;
}

int SearchTree::pruneTree(TreeNode& root)
{
	if (root.isLeaf())
		return false;

	int toDelete = 0;
	auto& children = root.children->nodes;

	if (children.size() > root.size)
		toDelete += children.size() - root.size;

	root.foreachChild([&](TreeNode& child)
	{
		if (pruneTree(child))		
			++toDelete;
	});

	if (root.size - toDelete <= 0 || children.size() < 1)
	{
		root.deallocateChildren();
	}
	else
	{
		//children.erase(children.end() - toDelete, children.end());
		//root.size = children.size();
		//root.deallocateRange(children.size() - toDelete, children.size());
		
		//if (root.size < children.capacity() / 3)
		//	children.shrink_to_fit();
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

