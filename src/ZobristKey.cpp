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

	// stores one hash key (i.e., a random 64 bit integer) for each coloured type of piece on each square
	uint64_t pieceHashKeys[12][64];

	// stores a hash key that is used whenever white makes a move
	uint64_t sideToPlayHashKey;

	// stores a hash key for each square. although not every square can have an en passant square set on it, 
	// giving the array a size of 64 allows for easy and efficient indexing
	uint64_t enpassantHashKeys[64];

	// stores 16 hash keys, one for each combination of castle privileges that a board can have at once 
	// (4 bits for castle privileges, 2 possible states per bit, 2^4 = 16)
	uint64_t castleHashKeys[16];

	// uses the c++ standard library to generate a 64 bit integer
	uint64_t getRandom64()
	{
		return (rand()) | ((uint64_t)rand() << 16) | ((uint64_t)rand() << 32) | ((uint64_t)rand() << 48);
	}

	// initializes all of the hash key tables with random 64 bit integers
	void init()
	{
		for (int pieceType = 0; pieceType < 12; pieceType++)
			for (int square = 0; square < 64; square++)
				pieceHashKeys[pieceType][square] = getRandom64();

		for (int castle = 0; castle < 16; castle++)
			castleHashKeys[castle] = getRandom64();

		for (int square = 0; square < 64; square++)
			enpassantHashKeys[square] = getRandom64();

		sideToPlayHashKey = getRandom64();
	}

	// returns the hash key for the piece on the given square
	// does so by determining the colour and type of piece that is on that square, 
	// then returning the appropriate hash key from the pieces hash table
	// this override of the function is used for zobrist key generation
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

	// returns a newly generated zobrist key
	zkey generate(ChessPosition* chessPosition)
	{
		zkey zobristKey = 0;

		// XOR all of the pieces on the board's hash keys
		for (int square = 0; square < 64; square++)
		{
			if (BB::boardSquares[square] & chessPosition->emptyBB) continue;

			zobristKey ^= getPieceHashKey(chessPosition, square);
		}

		// XOR the hash key for the castle privileges of the current position
		zobristKey ^= castleHashKeys[chessPosition->castlePrivileges];

		// XOR the hash key for the en passant square currently set in the position (if there are any at all)
		if (chessPosition->enPassantSquare != NO_SQUARE)
			zobristKey ^= enpassantHashKeys[chessPosition->enPassantSquare];

		// XOR the hash key for the side to move (XORING it only if the side to move is white)
		if (chessPosition->sideToMove == SIDE_BLACK)
			zobristKey ^= sideToPlayHashKey;

		return zobristKey;
	}
}