#include <random>

#include "DataTypes.h"
#include "ZobristKeyGenerator.h"

enum ColourAndPieceTypes
{
	WHITE_PAWN,
	WHITE_ROOK,
	WHITE_BISHOP,
	WHITE_QUEEN,
	WHITE_KNIGHT,
	WHITE_KING,

	BLACK_PAWN,
	BLACK_ROOK,
	BLACK_BISHOP,
	BLACK_QUEEN,
	BLACK_KNIGHT,
	BLACK_KING,
};

uint64_t ZobristKeyGenerator::getRandom64()
{
	return (rand()) | (rand() << 16) | ((uint64_t)rand() << 32) | ((uint64_t)rand() << 48);
}

void ZobristKeyGenerator::initHashKeys()
{
	for (int pieceType = 0; pieceType < 12; pieceType++)
		for (int square = 0; square < 64; square++)
			mPieceHashKeys[pieceType][square] = getRandom64();

	// for the sake of the tranposition table, should we store the square of the enPassant instead of a bitboard? i think yes? 
	for (int castle = 0; castle < 16; castle++)
		mCastleHashKeys[castle] = getRandom64();

	for (int square = 0; square < 64; square++)
		mEnpassantHashKeys[square] = getRandom64();

	mSideToPlayHashKey = getRandom64();
}

uint64_t ZobristKeyGenerator::getPieceHashKeyForGeneration(ChessPosition* chessPosition, Byte square)
{
	if (BB::boardSquares[square] & chessPosition->whitePiecesBB)
	{
		if	    (BB::boardSquares[square] & chessPosition->whitePawnsBB)   return mPieceHashKeys[WHITE_PAWN][square];
		else if (BB::boardSquares[square] & chessPosition->whiteRooksBB)   return mPieceHashKeys[WHITE_ROOK][square];
		else if (BB::boardSquares[square] & chessPosition->whiteBishopsBB) return mPieceHashKeys[WHITE_BISHOP][square];
		else if (BB::boardSquares[square] & chessPosition->whiteQueensBB)  return mPieceHashKeys[WHITE_QUEEN][square];
		else if (BB::boardSquares[square] & chessPosition->whiteKnightsBB) return mPieceHashKeys[WHITE_KNIGHT][square];
		else if (BB::boardSquares[square] & chessPosition->whiteKingBB)	   return mPieceHashKeys[WHITE_KING][square];
	}
	else // the occupied square is a black piece...
	{
		if		(BB::boardSquares[square] & chessPosition->blackPawnsBB)   return mPieceHashKeys[BLACK_PAWN][square];
		else if (BB::boardSquares[square] & chessPosition->blackRooksBB)   return mPieceHashKeys[BLACK_ROOK][square];
		else if (BB::boardSquares[square] & chessPosition->blackBishopsBB) return mPieceHashKeys[BLACK_BISHOP][square];
		else if (BB::boardSquares[square] & chessPosition->blackQueensBB)  return mPieceHashKeys[BLACK_QUEEN][square];
		else if (BB::boardSquares[square] & chessPosition->blackKnightsBB) return mPieceHashKeys[BLACK_KNIGHT][square];
		else if (BB::boardSquares[square] & chessPosition->blackKingBB)	   return mPieceHashKeys[BLACK_KING][square];
	}
}

uint64_t ZobristKeyGenerator::getPieceHashKeyForUpdate(ChessPosition* chessPosition, Bitboard* pieceBB, Byte square)
{
	if (!pieceBB)
		return 0;

	if (*pieceBB & chessPosition->whitePiecesBB)
	{
		if		(*pieceBB & chessPosition->whitePawnsBB)   return mPieceHashKeys[WHITE_PAWN][square];
		else if (*pieceBB & chessPosition->whiteRooksBB)   return mPieceHashKeys[WHITE_ROOK][square];
		else if (*pieceBB & chessPosition->whiteBishopsBB) return mPieceHashKeys[WHITE_BISHOP][square];
		else if (*pieceBB & chessPosition->whiteQueensBB)  return mPieceHashKeys[WHITE_QUEEN][square];
		else if (*pieceBB & chessPosition->whiteKnightsBB) return mPieceHashKeys[WHITE_KNIGHT][square];
		else if (*pieceBB & chessPosition->whiteKingBB)	   return mPieceHashKeys[WHITE_KING][square];
	}
	else // the occupied square is a black piece...
	{
		if		(*pieceBB & chessPosition->blackPawnsBB)   return mPieceHashKeys[BLACK_PAWN][square];
		else if (*pieceBB & chessPosition->blackRooksBB)   return mPieceHashKeys[BLACK_ROOK][square];
		else if (*pieceBB & chessPosition->blackBishopsBB) return mPieceHashKeys[BLACK_BISHOP][square];
		else if (*pieceBB & chessPosition->blackQueensBB)  return mPieceHashKeys[BLACK_QUEEN][square];
		else if (*pieceBB & chessPosition->blackKnightsBB) return mPieceHashKeys[BLACK_KNIGHT][square];
		else if (*pieceBB & chessPosition->blackKingBB)	   return mPieceHashKeys[BLACK_KING][square];
	}
}
ZobristKey ZobristKeyGenerator::generateKey(ChessPosition* chessPosition)
{
	ZobristKey zobristKey = 0;

	for (int square = 0; square < 64; square++)
	{
		if (BB::boardSquares[square] & chessPosition->emptyBB) continue;

		zobristKey ^= getPieceHashKeyForGeneration(chessPosition, square);
		zobristKey ^= getPieceHashKeyForGeneration(chessPosition, square);
	}

	zobristKey ^= mCastleHashKeys[chessPosition->castlePrivileges];
	zobristKey ^= mEnpassantHashKeys[chessPosition->enPassantSquare];
	if (!chessPosition->sideToMove)
		zobristKey ^= mSideToPlayHashKey;

	return zobristKey;
}

ZobristKey ZobristKeyGenerator::updateKey(ZobristKey zobristKey, ChessPosition* chessPosition, MoveData* moveData)
{
	zobristKey ^= getPieceHashKeyForUpdate(chessPosition, moveData->pieceBB, moveData->originSquare);
	zobristKey ^= getPieceHashKeyForUpdate(chessPosition, moveData->pieceBB, moveData->targetSquare);
	zobristKey ^= getPieceHashKeyForUpdate(chessPosition, moveData->capturedPieceBB, moveData->targetSquare);

	zobristKey ^= mCastleHashKeys[chessPosition->castlePrivileges];
	zobristKey ^= mEnpassantHashKeys[chessPosition->enPassantSquare];
	if (!chessPosition->sideToMove)
		zobristKey ^= mSideToPlayHashKey;

	return zobristKey;
}