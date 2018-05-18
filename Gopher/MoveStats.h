#pragma once


struct MoveStat
{
	int wins = 0;
	int visits = 0;

	//float winrate = 0.f;

	void clear() { wins = visits = 0; }
	//void clear() { winrate = visits = 0; }

	/*
	void addData(const int playouts, const int wins)
	{
		const int previousWins = static_cast<int>(winrate * visits);

		visits += playouts;
		winrate = static_cast<double>(previousWins + wins) / static_cast<double>(visits);
	}
	*/

	void addData(const int playouts, const int wincount)
	{
		visits += playouts;
		wins   += wincount;
	}
};
