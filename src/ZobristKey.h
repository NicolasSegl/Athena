#pragma once

#include "Bitboard.h"
#include "ChessPosition.h"
#include "MoveData.h"

// defines all of the functionality needed to generate and update zobrist keys
namespace ZobristKey
{
	// a zobrist key is simply a 64 bit number. more info about them can be found at
	// https://www.chessprogramming.org/Zobrist_Hashing
	typedef uint64_t zkey;
	
	void init();
	zkey generate(ChessPosition* chessPosition);
	zkey update(zkey zobristKey, ChessPosition* chessPosition, MoveData* moveData);
}