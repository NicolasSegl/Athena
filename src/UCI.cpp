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
	ChessGame chessGame;

	// response to "uci" command
	void respondUCI()
	{
		// identification
		std::cout << "id name Athena\n";
		std::cout << "id author Nicolas f\n";

		// options
		std::cout << "option name Move Overhead type spin default 10 min 0 max 5000\n";
		std::cout << "option name Threads type spin default 1 min 1 max 512\n";
		std::cout << "option name Hash type spin default 16 min 1 max 33554432\n";

		std::cout << "uciok\n";
	}

	// response to "isready" command
	void respondIsReady()
	{
		MoveGeneration::init();
		initBitsSetTable();
		Eval::init();
		ZobristKey::init();
		chessGame.init();
		std::cout << "readyok\n";
	}

	// response to commands involving a move made by the player ("position FEN (fen string) moves...")
	void respondPosition(const std::vector<std::string>& commandVec)
	{
		// index of the commandVec where moves start being described. if the FEN is startpos (the initial position), this index is 3
		// this can be seen by observing the uci command: position startpos moves ... (first move is at index 3)
		int movesCommandIndex = 3;

		if (commandVec[1] == "startpos") // index of 1 is the start of the FEN string
			chessGame.setPositionFEN(FEN_STARTING_STRING);
		else
		{
			std::string fenString = "";
			for (int i = 1; i <= 6; i++)
				fenString += commandVec[i] + " ";
			fenString.pop_back();

			chessGame.setPositionFEN(fenString);
			movesCommandIndex = 7;
		}

		// element at index 2 states simply "moves"
		for (int i = movesCommandIndex; i < commandVec.size(); i++)
			chessGame.makeMoveLAN(commandVec[i]);
	}

	// response to "go" command (telling the ai to search)
	void respondGo(const std::vector<std::string>& commandVec)
	{
		// extract all the data from the input and pass that into the function

		// the time athena has to search for a move. set to 99999 in case the uci gui doesn't specify the time (or time is unlimited)
		float athenaTime = 99999; // miliseconds

		for (int command = 1; command < commandVec.size(); command++)
		{
			if (commandVec[command] == "wtime" && chessGame.getSideToMove() == SIDE_WHITE) athenaTime = std::stof(commandVec[command + 1]);
			else if (commandVec[command] == "btime" && chessGame.getSideToMove() == SIDE_BLACK) athenaTime = std::stof(commandVec[command + 1]);
		}

		std::string bestMove = chessGame.findBestMove(chessGame.getSideToMove(), athenaTime);
		std::cout << "bestmove " << bestMove << "\n";
	}

	// ponderhit is a command. look into this
	void processCommand(const std::string& commandString)
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
		else if (commandVec[0] == "eval") // temporary debugging function used to test the current evaluation of the board
			std::cout << chessGame.getBoardEval() << std::endl;
		//else if (commandVec[0] == "ucinewgame")
		//	respondIsReady();
	}

	// may have to multithread so we can accept commands while Athena looks for a move
	void run()
	{
		while (true)
		{
			std::string uciInput;
			while (std::getline(std::cin, uciInput))
				processCommand(uciInput);
		}
	}
}