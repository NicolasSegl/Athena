#include <iostream>

#include "ChessGame.h"
#include "Constants.h"
#include "Outcomes.h"

void ChessGame::init()
{
	mBoard.init();
    mCheckOpeningBook = true;
}

std::string ChessGame::findBestMove(Colour side, float timeToMove)
{
    if (mCheckOpeningBook)
    {
        std::string openingMove = mAthena.getOpeningBookMove(&mBoard, mLANStringHistory);
        if (openingMove.size() > 0) // i.e., a move was found
            return openingMove;
    }

    mAthena.setColour(side);
    MoveData moveToMake = mAthena.search(&mBoard, timeToMove);
    return mBoard.getMoveLANString(&moveToMake);
}

void ChessGame::setPositionFEN(const std::string& fenString)
{
    // only check for opening moves when searching if the FEN string provided is the starting position
    if (fenString == FEN_STARTING_STRING)
        mCheckOpeningBook = true;
    else
        mCheckOpeningBook = false;
	mLANStringHistory.clear();
	mBoard.setPositionFEN(fenString);
}

void ChessGame::makeMoveLAN(const std::string& lanString)
{
	if (mBoard.makeMoveLAN(lanString))
		mLANStringHistory.push_back(lanString);
}
