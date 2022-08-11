#pragma once

#include "Bitboard.h"
#include "ChessPosition.h"
#include "MoveData.h"

typedef uint64_t ZobristKey;

// subject to a name change
class ZobristKeyGenerator
{
private:
	uint64_t getRandom64();

	uint64_t mPieceHashKeys[12][64];
	uint64_t mSideToPlayHashKey;
	uint64_t mEnpassantHashKeys[64];
	uint64_t mCastleHashKeys[16]; // 2^4. 1 bit set for each castle privilege

	uint64_t getPieceHashKeyForGeneration(ChessPosition* chessPosition, Byte square);
	uint64_t getPieceHashKeyForUpdate(ChessPosition* chessPosition, Bitboard* pieceBB, Byte square);
	
public:
	ZobristKeyGenerator() {}

	void initHashKeys();
	ZobristKey generateKey(ChessPosition* chessPosition);
	ZobristKey updateKey(ZobristKey zobristKey, ChessPosition* chessPosition, MoveData* moveData);
};