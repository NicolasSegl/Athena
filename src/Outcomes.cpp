#include "Board.h"
#include "ChessPosition.h"
#include "Outcomes.h"

namespace Outcomes
{
	// note that current ply will be the move that IS NOT YET RECORDED
	bool isThreefoldRepetition(ZobristKey* keyHistory, int currentPly)
	{
		// we only compare the LAST position with all the other postions in the history up to that point
		int repetitionCount = 1; // all positions recorded have occured at least once
		for (int i = 0; i < currentPly; i++)
			if (keyHistory[i] == keyHistory[currentPly])
				repetitionCount++;

		if (repetitionCount >= 3)
			return true;
		return false;
	}

	bool isFiftyMoveDraw(int fiftyMoveCounter) { return fiftyMoveCounter >= 100; }

	bool isInsufficientMaterial(ChessPosition position)
	{
		// no pawns
		if (!position.whitePawnsBB && !position.blackPawnsBB)
		{
			// lone kings
			if (position.occupiedBB & ~(position.whiteKingBB | position.blackKingBB) == 0)
				return true;
			//else if ()
		}
	}

	bool isDraw(Board* boardPtr)
	{
		return isThreefoldRepetition(boardPtr->getZobristKeyHistory(), boardPtr->getCurrentPly()) || isFiftyMoveDraw(boardPtr->getFiftyMoveCounter());
	}
}