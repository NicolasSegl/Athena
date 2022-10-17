#pragma once

#include "MoveData.h"
#include "ZobristKey.h"

struct TranspositionHashEntry
{
	enum HashFlagValues
	{
		NONEXISTENT,
		EXACT,
		LOWER_BOUND,
		UPPER_BOUND,
	};

	ZobristKey::zkey zobristKey = 0;
	int bestMoveIndex = -1;
	int depth;
	int eval;
	Byte hashFlag = HashFlagValues::NONEXISTENT;
};
