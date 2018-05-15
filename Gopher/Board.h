#pragma once
#include "Stone.h"
#include "Move.h"

// As is don't pass in any type needing memory 
// management
template<class Type, int size>
class FastList
{
	int mySize = 0;
	Type items[size]; // TODO: use aligned storage!
public:

	Type & operator[](const int idx)
	{
		return items[idx];
	}

	const Type & operator[](const int idx) const
	{
		return items[idx];
	}

	template<class... Args>
	void emplace_back(Args&& ...args)
	{
		new (&items[mySize++]) Type(std::forward<Args>(args)...);
	}

	void erase(int idx)
	{
		std::swap(items[idx], items[--mySize]);
	}

	// Find the first instance of val and erase it
	void findErase(int val)
	{
		for(int i = 0; i < mySize; ++i)
			if (val == items[i])
			{
				erase(i);
				return;
			}
	}

	Type pop_back()
	{
		return items[--mySize];
	}

	template<class Func>
	void foreach(Func&& func)
	{
		for (int i = 0; i < mySize; ++i)
			func(items[i]);
	}

	int size() const
	{
		return mySize;
	}
};

// Total liberty positions we keep track of
static constexpr int GroupLibCount = 10;

// If a groups liberties dip below this number 
// we'll try to refill them
static constexpr int GroupLibRefill = 5;
using groupId = int;

struct Group
{
	// Liberty count
	int libs;
	// Exact liberty locations
	coord lib[GroupLibCount];

	void addLib(const coord idx) { lib[libs++] = idx; }
};

struct GroupManager
{
	// Group id of stone at index idx
	groupId groupIds[BoardMaxIdx];

	// Next stone in group
	coord nextStone[BoardMaxIdx];

	// Info about groups, indexed by group id 
	// (which is idx of first group stone)
	Group groups[BoardMaxIdx];

	Group& groupInfoById(const groupId id) { return groups[id]; }
	const Group& groupInfoById(const groupId id) const { return groups[id]; }

	coord& groupNextStone(const coord idx) { return nextStone[idx]; }
	const coord& groupNextStone(const coord idx) const { return nextStone[idx]; }


	bool isGroupCaptured(const groupId id) const { return groups[id].libs == 0; }
};

struct Neighbors
{
	int n[Stone::MAX];

	void increment(Stone type) { ++n[type]; }
	void decrement(Stone type) { --n[type]; }
};

struct MoveStack;


// TODO: Move stuff around and check sizeof to find good alignment
struct Board
{
	int moveCount;

	// Capture info for all ston types
	int captures[Stone::MAX];

	// Info about squares (points) on the board
	// TODO: Make this char?
	int points[BoardMaxIdx];

	FastList<coord, BoardMaxIdx> free;

	// Number of neighbors a stone at idx has of each type
	Neighbors neighbors[BoardMaxIdx];

	GroupManager groups;

	// Store any ko move
	Move ko;
	Move lastMove;

	float komi;

public:
	void init();

	// Helper functions
	void printBoard() const;
	double scoreFast() const;
	double scoreReal() const;

	// Board state status functions
	int neighborCount(coord idx, Stone color) const;
	bool isEyeLike(coord idx, Stone color) const;
	bool isFalseEyelike(coord idx, Stone color) const;
	bool isOnePointEye(coord idx, Stone color) const;
	bool isValid(const Move& m) const;
	bool isValidNoSuicide(const Move& m) const;
	int immediateLibCount(coord idx) const;

	// Plays a random move if it can
	// TODO: Pass or use some heuristics so random moves are less "bad"
	coord playRandom(Stone color);
	bool tryRandomMove(Stone color, coord& idx, int freeIdx);

	// Update functions for things that happen with moves
	bool isGroupOneStone(const groupId id);
	void groupAddLibs(groupId group, coord idx);
	void groupFindLibs(Group& group, groupId groupid, coord idx);
	void groupRemoveLibs(groupId group, coord idx);
	groupId updateNeighbor(coord idx, const Move& m, groupId group);
	void removeStone(groupId gid, coord idx);

	// Move functions
	groupId moveNonEye(const Move& m);
	bool moveInEye(const Move& m);
	// This can fail, but not if it's passed a valid move
	bool makeMove(const Move& m);

	// Make the move from Gtp and test for pass/resigns
	bool makeMoveGtp(const Move& m);

	// Group functions
	groupId newGroup(coord idx);
	void addToGroup(groupId group, coord neighbor, coord newStone);
	void mergeGroups(groupId groupTo, groupId groupFrom);
	int groupCapture(groupId gid);
	groupId groupAt(const coord idx) const { return groups.groupIds[idx]; }
	groupId& groupAt(const coord idx) { return groups.groupIds[idx]; }

	const Group& groupInfoAt(const coord idx) const { return groups.groupInfoById(groupAt(idx)); }

	int at(int idx) const { return points[idx]; }
	int& at(int idx) { return points[idx]; }

	void setKomi(float num) { komi = num; }

	template<class F>
	void foreachPoint(F&& f);
	template<class F>
	void foreachPoint(F&& f) const;

	template<class F>
	void foreachNeighbor(int idx, F&& f);
	template<class F>
	void foreachNeighbor(int idx, F&& f) const;

	template<class F>
	void eachDiagonalNeighbor(int idx, F&& f);
	template<class F>
	void eachDiagonalNeighbor(int idx, F&& f) const;

	// Loop through the each member in group, 
	// llambda syntax should be [](int idx) { func }
	template<class F>
	void foreachInGroup(groupId id, F&& f);
	template<class F>
	void foreachInGroup(groupId id, F&& f) const;

	template<class F>
	void foreachFreePoint(F&& f) const;
};

// Llambda syntax is: [](int idx, int type){}
template<class F>
inline void Board::foreachPoint(F&& f)
{
	for (int i = 0; i < BoardMaxIdx; ++i)
		f(i, points[i]);
}

template<class F>
inline void Board::foreachPoint(F&& f) const
{
	for (int i = 0; i < BoardMaxIdx; ++i)
		f(i, points[i]);
}

// Don't call of points that are offboard
// Llambda syntax is: [](int idx, int type){}
template<class F>
inline void Board::foreachNeighbor(int idx, F&& f)
{
	f(idx - BoardRealSize, points[idx - BoardRealSize]);
	f(idx + 1, points[idx + 1]);
	f(idx + BoardRealSize, points[idx + BoardRealSize]);
	f(idx - 1, points[idx - 1]);
}
template<class F>
inline void Board::foreachNeighbor(int idx, F&& f) const
{
	f(idx - BoardRealSize, points[idx - BoardRealSize]);
	f(idx + 1, points[idx + 1]);
	f(idx + BoardRealSize, points[idx + BoardRealSize]);
	f(idx - 1, points[idx - 1]);
}

// Llambda syntax is: [](int idx, int type){}
template<class F>
inline void Board::eachDiagonalNeighbor(int idx, F&& f)
{
	f(idx + BoardRealSize + 1, points[idx + BoardRealSize + 1]);
	f(idx + BoardRealSize - 1, points[idx + BoardRealSize - 1]);
	f(idx - BoardRealSize + 1, points[idx - BoardRealSize + 1]);
	f(idx - BoardRealSize - 1, points[idx - BoardRealSize - 1]);
}

template<class F>
inline void Board::eachDiagonalNeighbor(int idx, F&& f) const
{
	f(idx + BoardRealSize + 1, points[idx + BoardRealSize + 1]);
	f(idx + BoardRealSize - 1, points[idx + BoardRealSize - 1]);
	f(idx - BoardRealSize + 1, points[idx - BoardRealSize + 1]);
	f(idx - BoardRealSize - 1, points[idx - BoardRealSize - 1]);
}

template<class F>
inline void Board::foreachInGroup(groupId id, F && f)
{
	while (id != 0)
	{
		f(id);
		id = groups.groupNextStone(id);
	}
}
template<class F>
inline void Board::foreachInGroup(groupId id, F && f) const
{
	while (id != 0)
	{
		f(id);
		id = groups.groupNextStone(id);
	}
}

template<class F>
inline void Board::foreachFreePoint(F && f) const
{
	for (int i = 0; i < free.size(); ++i)
		f(free[i]);
}

inline bool is8Adjacent(coord idx, coord other)
{
	return std::abs(idx - other) == 1 || std::abs(std::abs(idx - other) - BoardRealSize < 2);
}

inline bool isAdjacent(coord idx, coord other)
{
	return std::abs(idx - other) == 1 || std::abs(idx - other) == BoardRealSize;
}

