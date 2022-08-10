#pragma once

#include "ZobristKeyGenerator.h"
#include "MoveData.h"

struct TranspositionHashEntry
{
	enum HashFlagValues
	{
		NONEXISTENT,
		EXACT,
		LOWER_BOUND,
		UPPER_BOUND,
	};

	ZobristKey zobristKey;
	MoveData bestMove;
	short depth;
	int eval;
	Byte hashFlag;
};