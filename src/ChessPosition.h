#pragma once
#include "Bitboard.h"
#include "MoveData.h"

// the ai may not have to check all forms of a draw
// we can just say that if no moves pass through at all and the king is not in check, then it's a draw, return 0

struct ChessPosition
{
	// maybe make a struct in Bitboard.h with all 14 bitboards? then board can have one. would make writing
	// this sturct a hell of a lot easier. lots of rewriting tho. idk if that's goofy or not ?
	// in board, could call it mChessPosition ? it would contain the en passant bb, pieces bb, castle privileges, and the side to move? 

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

	Byte castlePrivileges = 0;
	Byte fiftyMoveCounter = 0;
	Byte enPassantSquare  = 0;

	Colour sideToMove = SIDE_WHITE;

	void reset() { *this = ChessPosition(); }
};