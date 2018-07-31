#include "Gtp.h"
#include "Net.h"
#include "Random.h"
#include "defines.h"
#include "Options.h"

#include <chrono>

/* // TODO: Must Build boost
#include <boost\program_options.hpp>

namespace po = boost::program_options;

void parseCmd(int argc, char * argv[])
{
	po::options_description desc("Allowed Options");
	desc.add_options()
		("validation", po::value<int>(), "Set validation characteristics")
	;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);
}
*/

int main(int argc, char * argv[])
{
	//parseCmd(argc, argv);

	Net::init();
	// Quick and dirty fix to make random number generator non-deterministic
	// for purposes of self-play
	//std::random_device rd;
	//std::uniform_int_distribution<int> dist(0, 99999);
	//for (int i = 0; i < dist(rd); ++i)
	//	Random::fastRandom(1);

	Gtp::mainLoop();

	return 0;
}

