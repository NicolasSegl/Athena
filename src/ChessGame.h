#pragma once

#include <string>
#include <vector>

#include "Athena.h"
#include "Bitboard.h"
#include "Board.h"
#include "Eval.h"
#include "MoveData.h"

// this class handles engine related commands from various UCI commands, such as "go" and "position"
class ChessGame
{
private:
	// a flag for if the engine should be using its opening book
    bool mCheckOpeningBook;

	Board mBoard;
    Athena mAthena;

	// contains the LAN move history of the match so far
	std::vector<std::string> mLANStringHistory;

public:
	void init();

	void setHashSize(int newSize) { mAthena.setTranspositionTableSize(newSize); }
	void setPositionFEN(const std::string& fenString);
	std::string findBestMove(Colour side, float timeToMove);
	void makeMoveLAN(const std::string& lanString);

	Colour getSideToMove() { return mBoard.currentPosition.sideToMove; 														   }
    int getBoardEval() 	   { return Eval::evaluatePosition(&mBoard, Eval::getMidgameValue(mBoard.currentPosition.occupiedBB)); }
};
