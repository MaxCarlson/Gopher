#include "Gtp.h"
#include "Board.h"
#include <sstream>
#include <iostream>
#include <string>

const std::string Commands[] =
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

template<class... Arg>
void gtpPrint(Arg&& ...arg)
{
	(std::cout << ... << arg);
}

void gtpEndReply() 
{
	std::cout << "\n\n";
}

void printId(int id, int status)
{
	if (status == GTP_OKAY)
		gtpPrint("=");
	else
		gtpPrint("?");
	
	if (id < 0)
		gtpPrint(" ");
	else
		gtpPrint(id); 
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

int printName(const std::string& str, int id);
int gtpProtocolVersion(const std::string& str, int id);
int engineVersion(const std::string& str, int id);

void mainLoop()
{
	Board board;
	std::string line;

	int id = 0;
	int isOkay = GTP_OKAY;

	gtpPrint("happy", "apples");

	while (isOkay && std::getline(std::cin, line))
	{
		// TODO: Loop this for multiple comments?
		auto firstComment = line.find('#');
		auto lastComment = line.find('#', firstComment);
		
		if(firstComment && lastComment)
			line.erase(firstComment, lastComment - firstComment + 1);

		std::istringstream is(line);

		std::string inputString;
		is >> inputString;

		id = std::atoi(inputString.c_str());

		if (id != 0)
		{

		}
		
		is >> inputString;
		

	}
}

int printName(const std::string& str, int id)
{
	return gtpSuccess(id, ENGINE_NAME);
}

int gtpProtocolVersion(const std::string & str, int id)
{
	return gtpSuccess(id, GTP_VERSION);
}

int engineVersion(const std::string & str, int id)
{
	return gtpSuccess(id, ENGINE_VERSION);
}
