#pragma once
#include "Stone.h"
#include "Move.h"

constexpr int BoardSize = 19;
constexpr int BoardRealSize = BoardSize + 2;
constexpr int BoardRealSize2 = BoardRealSize * BoardRealSize;
constexpr int BoardOffset = 2;
constexpr int BoardMaxIdx = (BoardSize + BoardOffset) * (BoardSize + BoardOffset);
constexpr int BoardMaxGroups = 228;//BoardSize * BoardSize * 2 / 3;


// As is don't pass in any type needing memory 
// management
template<class Type, int size>
class FastArray
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
		new (&items[mySize]) Type(std::forward<Args>(args)...);
		++mySize;
	}

	void erase(int idx)
	{
		std::swap(items[idx], items[--mySize]);
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

static constexpr int GroupLibCount = 10;
static constexpr int GroupLibRefill = 5;
using groupId = int;

struct Group
{
	// Liberty count
	int libs;
	// Exact liberty locations
	coord lib[GroupLibCount];
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

	Group& groupInfo(const coord idx) { return groups[groupIds[idx]]; }
	Group& groupInfoById(const groupId id) { return groupInfo(id); }
	coord& groupNextStone(const coord idx) { return nextStone[idx]; }
};

struct Neighbors
{
	int n[Stone::MAX];

	void increment(Stone type) { ++n[type]; }
};

struct Board
{
	int moveCount;

	// Capture info for all ston types
	int captures[Stone::MAX];

	// Info about squares (points) on the board
	int points[BoardMaxIdx];

	FastArray<coord, BoardMaxIdx> free;

	// Number of neighbors a stone at idx has of each type
	Neighbors neighbors[BoardMaxIdx];

	GroupManager groups;

	// Figure out what exactly needs to be stored here
	Move ko;

public:
	void init();

	int neighborCount(coord idx, Stone color) const;
	bool isEyeLike(coord idx, Stone color) const;
	bool isFalseEyelike(coord idx, Stone color) const;
	bool isOnePointEye(coord idx, Stone color) const;
	bool isValid(const Move& m) const;

	bool tryRandomMove(Stone color, coord& idx, int rng);

	// Returns 0 if can't play
	coord playRandom(Stone color);

	// Update functions for things that happen with moves
	bool isGroupOneStone(const groupId id);
	void groupAddLibs(groupId group, coord idx);
	void groupFindLibs(Group& group, groupId groupid, coord idx);
	void groupRemoveLibs(groupId group, coord idx);
	groupId updateNeighbor(coord idx, const Move& m, groupId group);

	// Move functions
	void moveNonEye(const Move& m);
	void makeMove(const Move& m);

	// Group functions
	void addToGroup(groupId group, coord neighbor, coord newStone);
	inline groupId groupAt(const coord idx) const { return groups.groupIds[idx]; }
	inline groupId& groupAt(const coord idx) { return groups.groupIds[idx]; }

	friend void printBoard(const Board& board);

	inline int at(int idx) const { return points[idx]; }
	inline int& at(int idx) { return points[idx]; }

	template<class F>
	void foreachPoint(F&& f);

	template<class F>
	void foreachNeighbor(int idx, F&& f);

	template<class F>
	void eachDiagonalNeighbor(int idx, F&& f);

	template<class F>
	void eachDiagonalNeighbor(int idx, F&& f) const { eachDiagonalNeighbor(idx, f); };

	// Loop through the each member in group, 
	// llambda syntax should be [](int idx) { func }
	template<class F>
	void foreachInGroup(groupId id, F&& f);
};

// Llambda syntax is: [](int idx, int type){}
template<class F>
inline void Board::foreachPoint(F&& f)
{
	for (int i = 0; i < BoardMaxIdx; ++i)
		f(i, points[i]);
}

// Don't call of points that are offboard
// Llambda syntax is: [](int idx, int type){}
template<class F>
inline void Board::foreachNeighbor(int idx, F&& f)
{
	f(idx - 1, points[idx - 1]);
	f(idx + 1, points[idx + 1]);
	f(idx - BoardRealSize, points[idx - BoardRealSize]);
	f(idx + BoardRealSize, points[idx + BoardRealSize]);
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
inline void Board::foreachInGroup(groupId id, F && f)
{
	while (id != 0)
	{
		f(id);
		id = groups.groupNextStone(id);
	}
}


