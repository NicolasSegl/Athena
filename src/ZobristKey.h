#pragma once

#include "Bitboard.h"
#include "ChessPosition.h"
#include "MoveData.h"

namespace ZobristKey
{
	typedef uint64_t zkey;
	
	void init();
	zkey generate(ChessPosition* chessPosition);
	zkey update(zkey zobristKey, ChessPosition* chessPosition, MoveData* moveData);
}