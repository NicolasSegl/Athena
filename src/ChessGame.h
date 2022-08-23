#pragma once

#include <string>
#include <vector>

#include "Athena.h"
#include "Bitboard.h"
#include "Board.h"
#include "MoveData.h"

// maybe abstract this into more classes after it starts working
class ChessGame
{
private:
    bool mCheckOpeningBook;
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
