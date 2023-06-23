#pragma once

#include "MoveData.h"
#include "ZobristKey.h"

// this structure contains all of the information needed to utilize the full functionality of a transposition table
struct TranspositionHashEntry
{
	enum HashFlagValues
	{
		NONEXISTENT,
		EXACT,
		LOWER_BOUND,
		UPPER_BOUND,
	};

	// contains the zobrist key of the position that is getting searched. this is used to verify that other
	// positions actually match the entry's (which would allow us to use the information in the table entry)
	ZobristKey::zkey zobristKey = 0;

	// stores the origin square of the best move found during the search (used for move ordering)
	Byte bestMoveOriginSquare = 255;

	// stores the depth of the search (i.e., how far it searched down the tree of possible moves from the position)
	int depth = 0;

	// stores the evaluation of the position's search at the depth defined above
	int eval = 0;

	// used for determining whether or not the entry was made after an alpha/beta cutoff (or neither)
	Byte hashFlag = HashFlagValues::NONEXISTENT;
};