#include <iostream>
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
		return (rand()) | ((uint64_t)rand() << 16) | ((uint64_t)rand() << 32) | ((uint64_t)rand() << 48);
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

	uint64_t getPieceHashKey(ChessPosition* chessPosition, Byte square)
	{
		uint64_t hashkey = 0;

		if (BB::boardSquares[square] & chessPosition->whitePiecesBB)
		{
			if	    (BB::boardSquares[square] & chessPosition->whitePawnsBB)   hashkey |=  pieceHashKeys[WHITE_PAWN][square];
			else if (BB::boardSquares[square] & chessPosition->whiteRooksBB)   hashkey |=  pieceHashKeys[WHITE_ROOK][square];
			else if (BB::boardSquares[square] & chessPosition->whiteBishopsBB) hashkey |=  pieceHashKeys[WHITE_BISHOP][square];
			else if (BB::boardSquares[square] & chessPosition->whiteQueensBB)  hashkey |=  pieceHashKeys[WHITE_QUEEN][square];
			else if (BB::boardSquares[square] & chessPosition->whiteKnightsBB) hashkey |=  pieceHashKeys[WHITE_KNIGHT][square];
			else if (BB::boardSquares[square] & chessPosition->whiteKingBB)	   hashkey |=  pieceHashKeys[WHITE_KING][square];
		}

		if (BB::boardSquares[square] & chessPosition->blackPiecesBB)
		{
			if		(BB::boardSquares[square] & chessPosition->blackPawnsBB)   hashkey |=  pieceHashKeys[BLACK_PAWN][square];
			else if (BB::boardSquares[square] & chessPosition->blackRooksBB)   hashkey |=  pieceHashKeys[BLACK_ROOK][square];
			else if (BB::boardSquares[square] & chessPosition->blackBishopsBB) hashkey |=  pieceHashKeys[BLACK_BISHOP][square];
			else if (BB::boardSquares[square] & chessPosition->blackQueensBB)  hashkey |=  pieceHashKeys[BLACK_QUEEN][square];
			else if (BB::boardSquares[square] & chessPosition->blackKnightsBB) hashkey |=  pieceHashKeys[BLACK_KNIGHT][square];
			else if (BB::boardSquares[square] & chessPosition->blackKingBB)	   hashkey |=  pieceHashKeys[BLACK_KING][square];
		}

		return hashkey;
	}

	uint getPieceHashKey(ChessPosition* chessPosition, Bitboard* pieceBB, Byte square)
	{
		uint64_t hashkey = 0;

		if (*pieceBB & chessPosition->whitePiecesBB)
		{
			if	    (*pieceBB & chessPosition->whitePawnsBB)   hashkey |=  pieceHashKeys[WHITE_PAWN][square];
			else if (*pieceBB & chessPosition->whiteRooksBB)   hashkey |=  pieceHashKeys[WHITE_ROOK][square];
			else if (*pieceBB & chessPosition->whiteBishopsBB) hashkey |=  pieceHashKeys[WHITE_BISHOP][square];
			else if (*pieceBB & chessPosition->whiteQueensBB)  hashkey |=  pieceHashKeys[WHITE_QUEEN][square];
			else if (*pieceBB & chessPosition->whiteKnightsBB) hashkey |=  pieceHashKeys[WHITE_KNIGHT][square];
			else if (*pieceBB & chessPosition->whiteKingBB)	   hashkey |=  pieceHashKeys[WHITE_KING][square];
		}

		if (*pieceBB & chessPosition->blackPiecesBB)
		{
			if		(*pieceBB & chessPosition->blackPawnsBB)   hashkey |=  pieceHashKeys[BLACK_PAWN][square];
			else if (*pieceBB & chessPosition->blackRooksBB)   hashkey |=  pieceHashKeys[BLACK_ROOK][square];
			else if (*pieceBB & chessPosition->blackBishopsBB) hashkey |=  pieceHashKeys[BLACK_BISHOP][square];
			else if (*pieceBB & chessPosition->blackQueensBB)  hashkey |=  pieceHashKeys[BLACK_QUEEN][square];
			else if (*pieceBB & chessPosition->blackKnightsBB) hashkey |=  pieceHashKeys[BLACK_KNIGHT][square];
			else if (*pieceBB & chessPosition->blackKingBB)	   hashkey |=  pieceHashKeys[BLACK_KING][square];
		}

		return hashkey;
	}

	zkey generate(ChessPosition* chessPosition)
	{
		zkey zobristKey = 0;

		for (int square = 0; square < 64; square++)
		{
			if (BB::boardSquares[square] & chessPosition->emptyBB) continue;

			zobristKey ^= getPieceHashKey(chessPosition, square);
		}

		zobristKey ^= castleHashKeys[chessPosition->castlePrivileges];
		zobristKey ^= enpassantHashKeys[chessPosition->enPassantSquare];
		if (!chessPosition->sideToMove)
			zobristKey ^= sideToPlayHashKey;

		return zobristKey;
	}

	zkey update(zkey zobristKey, ChessPosition* chessPosition, MoveData* moveData)
	{
		// std::cout << "before anything: " << zobristKey << std::endl;
		if (moveData->pieceBB)
		{
			zobristKey = zobristKey ^ getPieceHashKey(chessPosition, moveData->pieceBB, moveData->originSquare);
			std::cout << "hash key = " << getPieceHashKey(chessPosition, moveData->pieceBB, moveData->originSquare) 
					  << "xored: " << (zobristKey ^ getPieceHashKey(chessPosition, moveData->pieceBB, moveData->originSquare)) << std::endl;
			zobristKey ^= getPieceHashKey(chessPosition, moveData->pieceBB, moveData->targetSquare);
			// std::cout << "after target square: " << zobristKey << std::endl;
		}
		if (moveData->capturedPieceBB)
			zobristKey ^= getPieceHashKey(chessPosition, moveData->capturedPieceBB, moveData->targetSquare);

		zobristKey ^= castleHashKeys[chessPosition->castlePrivileges];
		zobristKey ^= enpassantHashKeys[chessPosition->enPassantSquare];
		if (!chessPosition->sideToMove)
			zobristKey ^= sideToPlayHashKey;

		return zobristKey;
	}
}