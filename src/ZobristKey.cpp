#include <random>

#include "DataTypes.h"
#include "ZobristKey.h"

namespace ZobristKey
{
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

		// put these in the c++ file! then use the init function :)
	uint64_t pieceHashKeys[12][64];
	uint64_t sideToPlayHashKey;
	uint64_t enpassantHashKeys[64];
	uint64_t castleHashKeys[16]; // 2^4. 1 bit set for each castle privilege

	uint64_t getRandom64()
	{
		return (rand()) | (rand() << 16) | ((uint64_t)rand() << 32) | ((uint64_t)rand() << 48);
	}

	void init()
	{
		for (int pieceType = 0; pieceType < 12; pieceType++)
			for (int square = 0; square < 64; square++)
				pieceHashKeys[pieceType][square] = getRandom64();

		// for the sake of the tranposition table, should we store the square of the enPassant instead of a bitboard? i think yes? 
		for (int castle = 0; castle < 16; castle++)
			castleHashKeys[castle] = getRandom64();

		for (int square = 0; square < 64; square++)
			enpassantHashKeys[square] = getRandom64();

		sideToPlayHashKey = getRandom64();
	}

	uint64_t getPieceHashKeyForGeneration(ChessPosition* chessPosition, Byte square)
	{
		if (BB::boardSquares[square] & chessPosition->whitePiecesBB)
		{
			if	    (BB::boardSquares[square] & chessPosition->whitePawnsBB)   return pieceHashKeys[WHITE_PAWN][square];
			else if (BB::boardSquares[square] & chessPosition->whiteRooksBB)   return pieceHashKeys[WHITE_ROOK][square];
			else if (BB::boardSquares[square] & chessPosition->whiteBishopsBB) return pieceHashKeys[WHITE_BISHOP][square];
			else if (BB::boardSquares[square] & chessPosition->whiteQueensBB)  return pieceHashKeys[WHITE_QUEEN][square];
			else if (BB::boardSquares[square] & chessPosition->whiteKnightsBB) return pieceHashKeys[WHITE_KNIGHT][square];
			else if (BB::boardSquares[square] & chessPosition->whiteKingBB)	   return pieceHashKeys[WHITE_KING][square];
		}
		else // the occupied square is a black piece...
		{
			if		(BB::boardSquares[square] & chessPosition->blackPawnsBB)   return pieceHashKeys[BLACK_PAWN][square];
			else if (BB::boardSquares[square] & chessPosition->blackRooksBB)   return pieceHashKeys[BLACK_ROOK][square];
			else if (BB::boardSquares[square] & chessPosition->blackBishopsBB) return pieceHashKeys[BLACK_BISHOP][square];
			else if (BB::boardSquares[square] & chessPosition->blackQueensBB)  return pieceHashKeys[BLACK_QUEEN][square];
			else if (BB::boardSquares[square] & chessPosition->blackKnightsBB) return pieceHashKeys[BLACK_KNIGHT][square];
			else if (BB::boardSquares[square] & chessPosition->blackKingBB)	   return pieceHashKeys[BLACK_KING][square];
		}
	}

	uint64_t getPieceHashKeyForUpdate(ChessPosition* chessPosition, Bitboard* pieceBB, Byte square)
	{
		if (!pieceBB)
			return 0;

		if (*pieceBB & chessPosition->whitePiecesBB)
		{
			if		(*pieceBB & chessPosition->whitePawnsBB)   return pieceHashKeys[WHITE_PAWN][square];
			else if (*pieceBB & chessPosition->whiteRooksBB)   return pieceHashKeys[WHITE_ROOK][square];
			else if (*pieceBB & chessPosition->whiteBishopsBB) return pieceHashKeys[WHITE_BISHOP][square];
			else if (*pieceBB & chessPosition->whiteQueensBB)  return pieceHashKeys[WHITE_QUEEN][square];
			else if (*pieceBB & chessPosition->whiteKnightsBB) return pieceHashKeys[WHITE_KNIGHT][square];
			else if (*pieceBB & chessPosition->whiteKingBB)	   return pieceHashKeys[WHITE_KING][square];
		}
		else // the occupied square is a black piece...
		{
			if		(*pieceBB & chessPosition->blackPawnsBB)   return pieceHashKeys[BLACK_PAWN][square];
			else if (*pieceBB & chessPosition->blackRooksBB)   return pieceHashKeys[BLACK_ROOK][square];
			else if (*pieceBB & chessPosition->blackBishopsBB) return pieceHashKeys[BLACK_BISHOP][square];
			else if (*pieceBB & chessPosition->blackQueensBB)  return pieceHashKeys[BLACK_QUEEN][square];
			else if (*pieceBB & chessPosition->blackKnightsBB) return pieceHashKeys[BLACK_KNIGHT][square];
			else if (*pieceBB & chessPosition->blackKingBB)	   return pieceHashKeys[BLACK_KING][square];
		}
	}

	zkey generate(ChessPosition* chessPosition)
	{
		zkey zobristKey = 0;

		for (int square = 0; square < 64; square++)
		{
			if (BB::boardSquares[square] & chessPosition->emptyBB) continue;

			zobristKey ^= getPieceHashKeyForGeneration(chessPosition, square);
		}

		zobristKey ^= castleHashKeys[chessPosition->castlePrivileges];
		zobristKey ^= enpassantHashKeys[chessPosition->enPassantSquare];
		if (!chessPosition->sideToMove)
			zobristKey ^= sideToPlayHashKey;

		return zobristKey;
	}

	zkey update(zkey zobristKey, ChessPosition* chessPosition, MoveData* moveData)
	{
		zobristKey ^= getPieceHashKeyForUpdate(chessPosition, moveData->pieceBB, moveData->originSquare);
		zobristKey ^= getPieceHashKeyForUpdate(chessPosition, moveData->pieceBB, moveData->targetSquare);
		zobristKey ^= getPieceHashKeyForUpdate(chessPosition, moveData->capturedPieceBB, moveData->targetSquare);

		zobristKey ^= castleHashKeys[chessPosition->castlePrivileges];
		zobristKey ^= enpassantHashKeys[chessPosition->enPassantSquare];
		if (!chessPosition->sideToMove)
			zobristKey ^= sideToPlayHashKey;

		return zobristKey;
	}
}