#include "Gtp.h"
#include <cctype>
#include <sstream>
#include <iostream>
#include <string>
#include <functional>
#include <map>
#include <array>

std::array<std::string, 11> Commands =
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
	"quit"
};

constexpr int GTP_VERSION = 2;
constexpr int GTP_OKAY = 1;
constexpr int GTP_FAIL = 0;
constexpr double ENGINE_VERSION = 0.01;
static const std::string ENGINE_NAME = "Gopher";

namespace Gtp
{


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

Board board;
std::map<std::string, std::function<int(std::istringstream&, int)>> options;

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
	float newKomi = std::stof(is.str()); 
	printId(id, GTP_OKAY);
	board.setKomi(newKomi);

	return gtpSuccess(id, "");
}

int getIdxFromGtp(const std::string& s)
{
	constexpr int aa = 'a';
	int x = (std::tolower(s[0]) - 97) + 1; 

	std::stringstream ss;
	for (int i = 1; i < s.size(); ++i)
		ss << s[i];

	int y = std::stoi(ss.str()) + 1;

	return xyToIdx(x, y);
}

int play(std::istringstream& is, int id)
{
	auto npos = std::string::npos;

	std::string tmp;
	is >> tmp;
	Stone color = (tmp.find('w') != npos || tmp.find('W') != npos) 
				? Stone::WHITE : Stone::BLACK;

	// TODO: Add pass and resign

	is >> tmp;
	int idx = getIdxFromGtp(tmp);

	Move m = { idx, color };

	if (!board.isValid(m))
		return gtpFailure(id, "invalid coordinates for move");

	board.makeMove(m);

	return gtpSuccess(id, "");
}

int setBoardSize(std::istringstream& is, int id)
{
	return 0;
}

int clearBoard(std::istringstream& is, int id)
{
	return 0;
}

int generateMove(std::istringstream& is, int id)
{
	return 0;
}

int quitGtp(std::istringstream& is, int id)
{
	return 0;
}

}
