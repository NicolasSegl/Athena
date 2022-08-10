#pragma once

#include "ZobristKeyGenerator.h"

namespace Outcomes
{
	// to see if it's a three repetition draw:
	// check if the colour bitboards are equal
	// then check each individual

	bool isThreefoldRepetition(ZobristKey* keyHistory, int currentPly);
	bool isFiftyMoveDraw(int fiftyMoveCounter);
};