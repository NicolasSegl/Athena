#pragma once

#include <cinttypes>

typedef uint64_t Bitboard;

constexpr bool SIDE_WHITE = false;
constexpr bool SIDE_BLACK = true;

// Bit Board namespace
namespace BB
{
	enum File
	{
		FILE_A,
		FILE_B,
		FILE_C,
		FILE_D,
		FILE_E,
		FILE_F,
		FILE_G,
		FILE_H,
	};

	enum Rank
	{
		RANK_FIRST,
		RANK_SECOND,
		RANK_THIRD,
		RANK_FOURTH,
		RANK_FIFTH,
		RANK_SIXTH,
		RANK_SEVENTH,
		RANK_EIGHTH,
	};

	namespace Constants
	{
		/* predefined bitboards for piece starting positions (little endian file-rank mapping) */

		// white pieces
		const Bitboard cWPawnsStartBB	= 65280;
		const Bitboard cWRooksStartBB	= 129;
		const Bitboard cWKnightsStartBB = 66;
		const Bitboard cWBishopsStartBB = 36;
		const Bitboard cWQueensStartBB  = 8;
		const Bitboard cWKingStartBB    = 16;

		// black pieces
		const Bitboard cBPawnsStartBB   = 71776119061217280;
		const Bitboard cBRooksStartBB   = 9295429630892703744;
		const Bitboard cBKnightsStartBB = 4755801206503243776;
		const Bitboard cBBishopsStartBB = 2594073385365405696;
		const Bitboard cBQueensStartBB  = 576460752303423488;
		const Bitboard cBKingStartBB    = 1152921504606846976;
	}

    extern Bitboard boardSquares[64];
    extern Bitboard fileClear[8]; // the vertical
    extern Bitboard rankClear[8]; // the horizontal

    void initialize();
    void printBitboard(Bitboard bitboard);
}
