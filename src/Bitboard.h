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

    extern Bitboard boardSquares[64];
    extern Bitboard fileClear[8]; // the vertical
    extern Bitboard rankClear[8]; // the horizontal

    void initialize();
    void printBitboard(Bitboard bitboard);
	int getLSB(Bitboard bb); // get least significant bit
	int getMSB(Bitboard bb); // get most significant bit
}
