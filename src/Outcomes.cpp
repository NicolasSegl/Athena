#include "Board.h"
#include "ChessPosition.h"
#include "Outcomes.h"

namespace Outcomes
{
	// checks for threefold repetition by comparing the zobrist key for the current position to all the previous
	// zobrist keys in the game's history
	bool isThreefoldRepetition(ZobristKey::zkey* keyHistory, int currentPly)
	{
		// we only compare the LAST position with all the other postions in the history up to that point
		int repetitionCount = 1; // all positions recorded have occured at least once
		for (int i = 0; i < currentPly; i++)
			if (keyHistory[i] == keyHistory[currentPly])
				repetitionCount++;

		// return true if the position was repeated 3 times
		if (repetitionCount >= 3)
			return true;
		return false;
	}

	// returns true if there have been fifty full moves with no pawn moves or captures
	bool isFiftyMoveDraw(int fiftyMoveCounter) { return fiftyMoveCounter >= 100; }

	// returns true if the position is a draw (either by threefold repetition or fifty move draw)
	// TODO: consider also insignificant material draws
	bool isDraw(Board* boardPtr)
	{
		return isThreefoldRepetition(boardPtr->getZobristKeyHistory(), boardPtr->getCurrentPly()) || isFiftyMoveDraw(boardPtr->getFiftyMoveCounter());
	}
}