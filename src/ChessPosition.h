#pragma once
#include "Bitboard.h"
#include "MoveData.h"

// this struct abstracts all of the information about a chess position that is provided by a FEN string
struct ChessPosition
{
	// white piece bitboards
	Bitboard whitePiecesBB  = 0;
	Bitboard whitePawnsBB   = 0;
	Bitboard whiteRooksBB   = 0;
	Bitboard whiteKnightsBB = 0;
	Bitboard whiteBishopsBB = 0;
	Bitboard whiteQueensBB  = 0;
	Bitboard whiteKingBB	= 0;

	// black piece bitboards
	Bitboard blackPiecesBB  = 0;
	Bitboard blackPawnsBB   = 0;
	Bitboard blackRooksBB   = 0;
	Bitboard blackKnightsBB = 0;
	Bitboard blackBishopsBB = 0;
	Bitboard blackQueensBB  = 0;
	Bitboard blackKingBB    = 0;

	Bitboard occupiedBB  = 0;
	Bitboard emptyBB	 = 0;

	// bit-encoded Byte, with the first 4 bits corresponding to whether white or black can still complete a long or
	// short castle. which bits correspond to what can be found in MoveData's CastlePrivileges enum
	Byte castlePrivileges = 0;

	Byte fiftyMoveCounter = 0;
	Byte enPassantSquare  = 0;

	Colour sideToMove = SIDE_WHITE;

	// this function resets all of the data in the instance to the default values above
	void reset() { *this = ChessPosition(); }
};