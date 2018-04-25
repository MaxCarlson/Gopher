#pragma once


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