#include "Gtp.h"
#include "GameState.h"
#include "Search.h"

#include <cctype>
#include <sstream>
#include <iostream>
#include <string>
#include <functional>
#include <map>
#include <array>

std::array<std::string, 14> Commands =
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
	"final_score",
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
	GameState stateStack;

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
	options.emplace(Commands[10], finalScore);
	options.emplace(Commands[11], quitGtp);
	options.emplace(Commands[12], gtpPrintBoard);
	options.emplace(Commands[13], playSelf);
}

void mainLoop()
{
	// Initialize board, root state, and tree
	// TODO: Look into loading games from SGF and so forth
	board.init();
	stateStack.makeMove(board);
	Tree::initRoot(board, stateStack, BLACK);

	buildCommandsMap(options);

	int id = 0;
	std::string line;
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
			gtpFailure(id, "Unknown command! ", inputString);

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
	if (s == "Pass" || s == "pass")
		return Pass;

	else if (s == "Resign" || s == "resign")
		return Resign;

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

	is >> tmp;
	int idx = getIdxFromGtp(tmp);
	Move m	= { idx, color };

	// We have to do some extra work here in case
	// user is playing the same color move 2x or more
	// times in a row
	Tree::verifyRoot(board, stateStack, color);

	if (isPass(m) || isResign(m))
		;
	else if (!board.isValid(m))
		return gtpFailure(id, "invalid coordinates for move");

	board.makeMoveGtp(m);
	stateStack.makeMove(board);

	if (isPass(m) || isResign(m))
		return gtpSuccess(id, isPass(m) ? "pass" : "resign");

	// Update the tree to the new rootstate
	Tree::updateRoot(board, stateStack, color, m.idx);

	return gtpSuccess(id, "");
}

int generateMove(std::istringstream& is, int id)
{
	std::string colorStr;
	is >> std::skipws >> colorStr;

	Stone color = gtpWOrB(colorStr);
	coord idx	= search.search(board, stateStack, color);
	Move m		= { idx, color };

	board.makeMoveGtp(m);
	stateStack.makeMove(board);

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
	stateStack.clear();
	return gtpSuccess(id, "");
}

int finalScore(std::istringstream& is, int id)
{
	int winner = Stone::WHITE;
	// Score from whites perspective
	// TODO: Probably need to try and find dead groups!?
	double score = board.scoreReal();

	if (score < 0.45)
	{
		score = -score;
		winner = flipColor(winner);
	}

	std::stringstream ss;
	ss << (winner == Stone::BLACK ? "B+" : "W+");

	int intScore = std::floor(score);
	double hp = score - static_cast<double>(intScore);

	ss << intScore;

	if (hp)
		ss << ".5";

	if (score == 0.0)
		ss.str() = "0";

	return gtpSuccess(id, ss.str());
}

int quitGtp(std::istringstream& is, int id)
{
	return gtpFailure(id, "Quitting");
}

int gtpPrintBoard(std::istringstream& is, int id)
{
	board.printBoard();

	return gtpSuccess(id);
}

// TODO: Rebuild for Net
int playSelf(std::istringstream& is, int id)
{
	int passes = 0;
	for (int i = 0; i < 10000; ++i)
	{

	}

	return gtpSuccess(id);
}

} // End Gtp::
