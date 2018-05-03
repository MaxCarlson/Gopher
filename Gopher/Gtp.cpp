#include "Gtp.h"
#include "MonteCarlo.h"
#include "Uct.h"

#include <cctype>
#include <sstream>
#include <iostream>
#include <string>
#include <functional>
#include <map>
#include <array>

std::array<std::string, 13> Commands =
{
	"name",
	"version",
	"protocol_version",
	"list_commands",
	"known_command",
	"komi",
	"play",
	"boardsize",
	"clear_board",
	"genmove",
	"quit",
	"print",
	"play_self"
};

constexpr int GTP_VERSION = 2;
constexpr int GTP_OKAY = 1;
constexpr int GTP_FAIL = 0;
constexpr double ENGINE_VERSION = 0.01;
static const std::string ENGINE_NAME = "Gopher";

namespace Gtp
{
	Board board;
	MoveStack moveStack;
	//Stone color = Stone::BLACK;
	std::map<std::string, std::function<int(std::istringstream&, int)>> options;

template<class... Arg>
void gtpPrint(Arg&& ...arg)
{
	(std::cout << ... << arg);
}

int gtpEndReply() 
{
	std::cout << "\n\n";
	return GTP_OKAY;
}

void printId(int id, int status)
{
	if (status == GTP_OKAY)
		gtpPrint("=");
	else
		gtpPrint("?");
	
	if (id <= 0)
		gtpPrint(" ");
	else
		gtpPrint(id, " "); 
}

template<class... Args>
int gtpSuccess(int id, Args&& ...args)
{
	printId(id, GTP_OKAY);
	gtpPrint(std::forward<Args>(args)...);
	gtpEndReply();
	return GTP_OKAY;
}

template<class... Args>
int gtpFailure(int id, Args&& ...args)
{
	printId(id, GTP_FAIL);
	gtpPrint(std::forward<Args>(args)...);
	gtpEndReply();
	return GTP_FAIL;
}

void gtpPanic()
{
	gtpPrint("! panic");
	gtpEndReply();
}


template<class Map>
void buildCommandsMap(Map& options)
{
	options.emplace(Commands[0], printName);
	options.emplace(Commands[1], engineVersion);
	options.emplace(Commands[2], gtpProtocolVersion);
	options.emplace(Commands[3], listCommands);
	options.emplace(Commands[4], knownCommand);
	options.emplace(Commands[5], setKomi);
	options.emplace(Commands[6], play);
	options.emplace(Commands[7], setBoardSize);
	options.emplace(Commands[8], clearBoard);
	options.emplace(Commands[9], generateMove);
	options.emplace(Commands[10], quitGtp);
	options.emplace(Commands[11], gtpPrintBoard);
	options.emplace(Commands[12], playSelf);
}

void mainLoop()
{
	board.init();

	buildCommandsMap(options);

	std::string line;

	int id = 0;
	int gtpStatus = GTP_OKAY;

	while (gtpStatus && std::getline(std::cin, line))
	{
		// TODO: Need to filter out comments

		std::istringstream is(line);

		std::string inputString;
		is >> inputString;

		id = std::atoi(inputString.c_str());

		if (id != 0)
			is >> inputString;
		
			
		auto findCommand = options.find(inputString);

		if (findCommand != options.end())
		{
			gtpStatus = findCommand->second(is, id);
		}
		else
			gtpFailure(id, "Uknown command! ", inputString);

		if (gtpStatus == GTP_FAIL)
			gtpPanic();
	}
}

int printName(std::istringstream& is, int id)
{
	return gtpSuccess(id, ENGINE_NAME);
}

int gtpProtocolVersion(std::istringstream& is, int id)
{
	return gtpSuccess(id, GTP_VERSION);
}

int engineVersion(std::istringstream& is, int id)
{
	return gtpSuccess(id, ENGINE_VERSION);
}

int listCommands(std::istringstream& is, int id)
{
	printId(id, GTP_OKAY);

	for (const auto& str : Commands)
	{
		gtpPrint(str);
		std::cout << '\n';
	}

	return gtpEndReply();
}

int knownCommand(std::istringstream& is, int id)
{
	auto findCommand = options.find(is.str());

	if (findCommand != options.end())
	{
		printId(id, GTP_OKAY);
		gtpPrint("true");
	}
	else
	{
		printId(id, GTP_FAIL);
		gtpPrint("false");
	}

	return gtpEndReply();
}

int setKomi(std::istringstream& is, int id)
{
	std::string kom;
	is >> kom;
	float newKomi = std::stof(kom); 
	printId(id, GTP_OKAY);
	board.setKomi(newKomi);

	return gtpSuccess(id);
}

// Convert gtp coordinates to our index
int getIdxFromGtp(const std::string& s)
{
	char tx = (std::tolower(s[0])); 

	// Fix any index at our greater than i
	if (tx >= 'i')
		tx -= 1;

	// Convert char to x axis coord (- 97).
	// our index is offset by a boundry of one offboard tile
	int x = tx - 97 + 1;

	std::stringstream ss;
	for (int i = 1; i < s.size(); ++i)
		ss << s[i];

	// TODO: Need to flip y coord along BoardRealSize, ie 1 becomes 19, 18 becomes 2

	int y = gtpFlipY( std::stoi(ss.str()));

	return xyToIdx(x, y);
}

// Get x and y for gtp from an idx
std::pair<char, int> gtpIdxToXY(coord idx)
{
	auto xy = idxToXY(idx);

	char x = (xy.first + 65) - 1;

	// No 'I' coordinate in go
	if (x >= 'I')
		x += 1;

	return { x, gtpFlipY(xy.second) };
}

Stone gtpWOrB(const std::string& s)
{
	static constexpr auto npos = std::string::npos;
	return (s.find('w') != npos || s.find('W') != npos)
		? Stone::WHITE : Stone::BLACK;
}

int play(std::istringstream& is, int id)
{
	std::string tmp;
	is >> tmp;
	Stone color = gtpWOrB(tmp);

	// TODO: Add pass and resign

	is >> tmp;
	int idx = getIdxFromGtp(tmp);

	Move m = { idx, color };

	if (!board.isValid(m))
		return gtpFailure(id, "invalid coordinates for move");

	board.makeMoveGtp(m);

	if (isPass(m) || isResign(m))
		return gtpSuccess(id, isPass(m) ? "pass" : "resign");

	// Rudimentry move stack
	moveStack.emplace_back(m);

	return gtpSuccess(id, "");
}

int generateMove(std::istringstream& is, int id)
{
	MonteCarlo monte{ board };

	std::string colorStr;
	is >> std::skipws >> colorStr;

	Stone color = gtpWOrB(colorStr);

	//coord idx = monte.genMove(color);
	coord idx = uct.search(board, color);


	Move m = { idx, color };

	board.makeMoveGtp(m);

	// Rudimentry move stack
	moveStack.emplace_back(m);

	if (isPass(m) || isResign(m))
		return gtpSuccess(id, isPass(m) ? "pass" : "resign");

	auto xy = gtpIdxToXY(idx);

	return gtpSuccess(id, xy.first, xy.second);
}

int setBoardSize(std::istringstream& is, int id)
{
	return gtpSuccess(id, "No resize possible yet!");
}

int clearBoard(std::istringstream& is, int id)
{
	board.init();
	return gtpSuccess(id, "");
}

int quitGtp(std::istringstream& is, int id)
{
	return gtpFailure(id, "Quitting");
}

int gtpPrintBoard(std::istringstream& is, int id)
{
	std::cout << "\n # \n";
	board.printBoard();
	std::cout << "\n # \n";

	return gtpSuccess(id);
}

// Mainly just for quick testing
int playSelf(std::istringstream& is, int id)
{
	const std::string playBlack = " b";
	const std::string playWhite = " w";

	for (int i = 0; i < 10000; ++i)
	{
		std::istringstream sb(playBlack);
		std::istringstream sw(playWhite);

		generateMove(i % 2 == 0 ? sb : sw, 0);

		board.printBoard();
	}

	return gtpSuccess(id);
}

} // End Gtp::
