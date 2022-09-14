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

	extern Bitboard eastFile[8];
	extern Bitboard westFile[8];
	extern Bitboard adjacentFiles[8];

    extern Bitboard boardSquares[64];

    extern Bitboard fileClear[8];
    extern Bitboard fileMask[8];

    extern Bitboard rankClear[8];
    extern Bitboard rankMask[8];

    void initialize();
    void printBitboard(Bitboard bitboard);
	int getLSB(Bitboard bb); // get least significant bit
	int getMSB(Bitboard bb); // get most significant bit
}
