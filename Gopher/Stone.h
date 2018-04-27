#pragma once
#include <string>

enum Stone
{
	NONE,
	BLACK,
	WHITE,
	OFFBOARD,
	MAX
};

inline Stone flipColor(Stone stone)
{
	static constexpr Stone flip[] = { NONE, WHITE, BLACK, OFFBOARD };
	return flip[stone];
}

inline Stone flipColor(int stone) { return flipColor(static_cast<Stone>(stone)); }

inline std::string printStone(Stone stone)
{
	switch (stone)
	{
	case Stone::NONE:
		return "None";
	case Stone::BLACK:
		return "Black";
	case Stone::WHITE:
		return "White";
	case Stone::OFFBOARD:
		return "Off Board";
	default:
		return "Stone out of bounds!";
	}
	return "Stone out of bounds!";
}