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

	ZobristKey::zkey zobristKey;
	MoveData bestMove;
	Byte depth;
	short eval;
	Byte hashFlag;
};
