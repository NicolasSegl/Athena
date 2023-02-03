#include <iostream>
#include <sstream>

#include "Constants.h"
#include "Eval.h"
#include "MoveGeneration.h"
#include "UCI.h"
#include "utils.h"
#include "ZobristKey.h"

namespace UCI
{
	// a global instance of the ChessGame class to be used for abstracting the engine and searching for moves
	// it is defined as a global variable here to prevent UCI from unnecessarily becoming a class
	ChessGame chessGame;

	// response to the "uci" command
	void respondUCI()
	{
		// identification
		std::cout << "id name Athena\n";
		std::cout << "id author Nicolas f\n";

		// options
		std::cout << "option name Move Overhead type spin default 10 min 0 max 5000\n";
		std::cout << "option name Threads type spin default 1 min 1 max 512\n";
		std::cout << "option name Hash type spin default 16 min 1 max 33554432\n";

		// response indicating that the engine is ready for the next command
		std::cout << "uciok\n";
	}

	// response to the "isready" command
	void respondIsReady()
	{
		// response indicating that the engine is ready for the next command
		std::cout << "readyok\n";
	}

	// response to commands involving either a move made by the player, or a position being imposed upon the board ("position FEN (fen string) moves...")
	// note that no cout response is needed for the GUI to know that the engine has interpreted this command
	void respondPosition(const std::vector<std::string>& commandVec)
	{
		// stores the index of the commandVec where LAN moves start. if the FEN is startpos (the initial position), this index is 3
		// this can be seen by observing the uci command: position startpos moves ... (first move would be at index 3)
		int movesCommandIndex = 3;

		// the command vector's index of 1 is the start of the FEN string
		// if it is "startpos", then we want to set the board's position using the actual FEN string for the starting position of a chess game
		// this FEN string is defined in Constants.h as FEN_STARTING_STRING
		if (commandVec[1] == "startpos")
			chessGame.setPositionFEN(FEN_STARTING_STRING);
		else
		{
			// if the FEN string being fed to the engine via UCI is not the starting position of a chess game,
			// then we'll need to get all 6 parts of the FEN string and then give it to the board
			std::string fenString = "";
			for (int i = 1; i <= 6; i++)
				fenString += commandVec[i] + " ";

			// popping the last value off of the FEN string we've put into an std::string will get rid of the final space at the end
			// (the for loop above puts a space at the end of each part, though the final one is unncessary and will mess up how the
			// Board class parses the FEN string)
			fenString.pop_back();

			chessGame.setPositionFEN(fenString);

			// because the FEN string took up extra elements of the commands vector, the index at which the LAN moves (if any)
			// starts is now at an index of 7
			movesCommandIndex = 7;
		}

		// iterate over any possible moves from the FEN string provided that have occured, and make them using the engine's abstractions
		for (int i = movesCommandIndex; i < commandVec.size(); i++)
			chessGame.makeMoveLAN(commandVec[i]);
	}

	// response to the "go" command
	void respondGo(const std::vector<std::string>& commandVec)
	{
		// the time athena has to search for a move. set to 99999 in case the uci gui doesn't specify the time (or time is unlimited)
		float athenaTime = 99999; // in miliseconds

		// iterate over the options given in the "go" call to get the amount of time that Athena has left on its timer
		for (int command = 1; command < commandVec.size(); command++)
		{
			if (commandVec[command] == "wtime" && chessGame.getSideToMove() == SIDE_WHITE) 		athenaTime = std::stof(commandVec[command + 1]);
			else if (commandVec[command] == "btime" && chessGame.getSideToMove() == SIDE_BLACK) athenaTime = std::stof(commandVec[command + 1]);
		}

		// find the best move (in LAN format) that Athena could make in the position
		std::string bestMove = chessGame.findBestMove(chessGame.getSideToMove(), athenaTime);

		// this communicates to the GUI what Athena's best move was, and allows the GUI to send more commands to the engine
		std::cout << "bestmove " << bestMove << "\n";
	}

	// processes the commands that the GUI sends to the engine
	void processCommand(const std::string& commandString)
	{
		// store the words in the command string into a vector, so that the words can be accessed by index
		std::vector<std::string> commandVec;
		splitString(commandString, commandVec, ' ');

		// the below if statements respond to various commands by calling their respective response function

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
		else if (commandVec[0] == "ucinewgame")
			respondIsReady();

		// this is a debugging function used to print the current evaluation of the board
		// it is not a UCI command
		else if (commandVec[0] == "eval")
			std::cout << chessGame.getBoardEval() << std::endl;
	}

	// waits on GUI input to the engine using the UCI interface, and provokes a response if and when necessary
	void run()
	{
		// the following calls to various init functions initializes the engine
		MoveGeneration::init();
		initBitsSetTable();
		Eval::init();
		ZobristKey::init();
		chessGame.init();

		while (true)
		{
			std::string uciInput;
			while (std::getline(std::cin, uciInput))
				processCommand(uciInput);
		}
	}
}