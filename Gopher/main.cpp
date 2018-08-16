#include "Gtp.h"
#include "Net.h"
#include "Random.h"
#include "defines.h"
#include "Options.h"
#include <chrono>
#include <boost\program_options.hpp>

namespace po = boost::program_options;

void parseCmd(int argc, char * argv[])
{
	po::options_description desc("Allowed Options");
	desc.add_options()
		("playouts,p",		po::value<int>(),	 "Set max playouts network will search per turn")
		("resign,r",		po::value<double>(), "% of evaluation at which engine should resign")
		("val,v",			po::value<bool>(),	 "Set validation")
		("vmax,m",			po::value<int>(),	 "Set max moves during validation")
		("noise,n",			po::value<int>(),    "Set the number of moves, move(0-n) where proportional noise is introduced into root move selection")
	;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if(vm.count("playouts"))
		std::cerr << "Playouts set to: " << (options.maxPlayouts = vm["playouts"].as<int>()) << '\n';
	if (vm.count("resign"))
		std::cerr << "Resign threshold set to: " << (options.resignThresh = vm["resign"].as<double>()) << '\n';
	if (vm.count("val"))
		std::cerr << "Validation set to: " << (options.validation = vm["val"].as<bool>()) << '\n';
	if (vm.count("vmax"))
		std::cerr << "Max validation moves set to: " << (options.valMaxMoves = vm["vmax"].as<int>()) << '\n';
	if (vm.count("noise"))
		std::cerr << "Noisy move count set to: " << (options.rngMovesNumber = vm["noise"].as<int>()) << '\n';
}


int main(int argc, char * argv[])
{
	parseCmd(argc, argv);

	Net::init();
	// Quick and dirty fix to make random number generator non-deterministic
	// for purposes of self-play
	std::random_device rd;
	std::uniform_int_distribution<int> dist(0, 9999);
	for (int i = 0; i < dist(rd); ++i)
		Random::fastRandom(1);

	Gtp::mainLoop();

	return 0;
}

