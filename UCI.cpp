#include "UCI.h"
#include "Constants.h"

#include <iostream>
#include <sstream>

#include "utils.h"

// response to "uci" command
void UCI::respondUCI()
{
	// identification
	std::cout << "id name Athena\n";
	std::cout << "id author Nicolas Seglenieks\n";

	// options
	std::cout << "option name Move Overhead type spin default 10 min 0 max 5000\n";
	std::cout << "option name Threads type spin default 1 min 1 max 512\n";
	std::cout << "option name Hash type spin default 16 min 1 max 33554432\n";

	std::cout << "uciok\n";
}

// response to "isready" command
void UCI::respondIsReady()
{
	std::cout << "readyok\n";
}

// response to commands involving a move made by the player ("position fen (fen string) moves...")
void UCI::respondPosition(const std::vector<std::string>& commandVec)
{
	std::string fenString = commandVec[1];

	if (fenString == "startpos")
		mCG.setPositionFEN(FEN::start);
	else
		mCG.setPositionFEN(fenString);

	// element at index 2 states simply "moves"
	for (int i = 3; i < commandVec.size(); i++)
		mCG.makeMoveLAN(commandVec[i]);

	// here set athena's colour
}

// response to "go" command (telling the ai to search)
void UCI::respondGo(const std::vector<std::string>& commandVec)
{
	// extract all the data from the input and pass that into the function

	// the time athena has to search for a move. set to 99999 in case the uci gui doesn't specify the time (or time is unlimited)
	float athenaTime = 99999; // miliseconds

	for (int command = 1; command < commandVec.size(); command++)
	{
		if		(commandVec[command] == "wtime" && mCG.getSideToMove() == SIDE_WHITE) athenaTime = std::stof(commandVec[command + 1]);
		else if (commandVec[command] == "btime" && mCG.getSideToMove() == SIDE_BLACK) athenaTime = std::stof(commandVec[command + 1]);
	}
	std::cout << "bestmove " << mCG.findBestMove(mCG.getSideToMove(), athenaTime) << "\n";
}

// ponderhit is a command. look into this
void UCI::processCommand(const std::string& commandString)
{
	std::vector<std::string> commandVec;
	splitString(commandString, commandVec, ' ');

	if (commandVec[0] == "uci")
		respondUCI();
	else if (commandVec[0] == "isready")
		respondIsReady();
	else if (commandVec[0] == "position")
		respondPosition(commandVec);
	else if (commandVec[0] == "go")
		respondGo(commandVec);
	else if (commandVec[0] == "quit")
		exit(0);
	//else if (commandVec[0] == "ucinewgame")
	//	respondIsReady();
}

// may have to multithread so we can accept commands while Athena looks for a move
void UCI::run()
{
	mCG.init();

	while (true)
	{
		std::string uciInput;
		while (std::getline(std::cin, uciInput))
			processCommand(uciInput);
	}
}

// probably the moves aren't updating the board properly as Athena was making completely stupid moves