#pragma once

#include <vector>
#include <string>

#include "Bitboard.h"
#include "Board.h"
#include "MoveData.h"
#include "Athena.h"

// maybe abstract this into more classes after it starts working
class ChessGame
{
private:
	Board mBoard;
    Athena mAthena;
	std::vector<std::string> mLANStringHistory;

public:
	void init();

	void setPositionFEN(const std::string& fenString);
	std::string findBestMove(Colour side, float timeToMove);
	void makeMoveLAN(const std::string& lanString);
	Colour getSideToMove() { return mBoard.currentPosition.sideToMove; }
};
