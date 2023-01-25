#include <iostream>

#include "ChessGame.h"
#include "Constants.h"
#include "Outcomes.h"

// initializes the board and opening book state
void ChessGame::init()
{
	mBoard.init();
    mCheckOpeningBook = true;
}

// this function uses Athena to return the best move it can find (in LAN format)
// the exception to this is when we are using the opening book (such as at the start of the game)
std::string ChessGame::findBestMove(Colour side, float timeToMove)
{
    if (mCheckOpeningBook)
    {
        // get the opening move as a LAN string
        std::string openingMove = mAthena.getOpeningBookMove(&mBoard, mLANStringHistory);

        // if the string size is greater than 0, it means that an opening move was found
        if (openingMove.size() > 0)
            return openingMove;
    }

    // if no opening move could be used, then use Athena to find the best move via minimax searching
    mAthena.setColour(side);
    MoveData moveToMake = mAthena.search(&mBoard, timeToMove);

    return mBoard.getMoveLANString(&moveToMake);
}

// uses an FEN string to set the board's position using the engine's abstractions
void ChessGame::setPositionFEN(const std::string& fenString)
{
    // if the FEN position we set it to isn't the starting position, then Athena won't use her opening book to find moves
    if (fenString == FEN_STARTING_STRING)
        mCheckOpeningBook = true;
    else
        mCheckOpeningBook = false;

    // erase any move history we had before resetting the board's position
	mLANStringHistory.clear();

	mBoard.setPositionFEN(fenString);
}

// make a move on the board using a move in the LAN format, adding the move to the game's history if it was legal
void ChessGame::makeMoveLAN(const std::string& lanString)
{
	if (mBoard.makeMoveLAN(lanString))
		mLANStringHistory.push_back(lanString);
}
