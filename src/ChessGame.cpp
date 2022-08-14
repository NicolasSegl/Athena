#include <iostream>

#include "ChessGame.h"
#include "Outcomes.h"

void ChessGame::init()
{
	mBoard.init();
	mAthena.activate();
}

std::string ChessGame::findBestMove(Colour side, float timeToMove)
{
	std::string openingMove = mAthena.getOpeningBookMove(&mBoard, mLANStringHistory);
	if (openingMove == "") 	// if no opening move is found
	{
		mAthena.setColour(side);
		MoveData moveToMake = mAthena.search(&mBoard, timeToMove);
		return mBoard.getMoveLANString(&moveToMake);
	}
	else
		return openingMove;
}

void ChessGame::setPositionFEN(const std::string& fenString)
{
	mLANStringHistory.clear();
	mBoard.setPositionFEN(fenString);
}

void ChessGame::makeMoveLAN(const std::string& lanString)
{
	if (mBoard.makeMoveLAN(lanString))
		mLANStringHistory.push_back(lanString);
}
