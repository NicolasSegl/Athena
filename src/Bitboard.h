#pragma once

#include <cinttypes>

typedef uint64_t Bitboard;

constexpr bool SIDE_WHITE = false;
constexpr bool SIDE_BLACK = true;

// this namespace definse all of the general purpose bitboards and bitboard functions that Athena uses
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

	// functions for bit shifting a bitboard in some direction as to get the corresponding resultant bitboard 
	inline Bitboard northEastOne(Bitboard bb)  { return bb << 9; }
	inline Bitboard northOne(Bitboard bb) 	   { return bb << 8; }
	inline Bitboard northWestOne(Bitboard bb)  { return bb << 7; }
	inline Bitboard westOne(Bitboard bb) 	   { return bb << 1; }
	inline Bitboard eastOne(Bitboard bb) 	   { return bb >> 1; }
	inline Bitboard southWestOne(Bitboard bb)  { return bb >> 9; }
	inline Bitboard southOne(Bitboard bb) 	   { return bb >> 8; }
	inline Bitboard southEastOne(Bitboard bb)  { return bb >> 7; }
	
	/*
		each element in the eastFile array is a Bitboard with one entire file set (except for the 8th element, as there is no file to the east of the H file)
		likewise, each element in the westFile array has Bitboards with just one file set (save for the 1st element, as there are no files to the west of the A file)

		each array has the file either one to the east or one to the west from the index given completely set to 1s
		for instance, accessing the 4th element of the eastFile array would return a Bitboard with the entire 5th file set to 1s

		the adjacentFiles array returns a Bitboard with both one to the east and one to the west files set to 1s (should it be possible. for instance, indexing the
		first element would mean that there would be no west file set, as there is no file to the west of the A file)
	*/
	extern Bitboard eastFile[8];
	extern Bitboard westFile[8];
	extern Bitboard adjacentFiles[8];

	// an array of Bitboards, with each element representing one individual square (each element has only 1 bit set in its entire Bitboard)
    extern Bitboard boardSquares[64];

	// an array that contains Bitboards with one file cleared (set to 0s) and the rest masked (set to 1s)
    extern Bitboard fileClear[8];

	// an array that contains Bitboards with one file masked (set to 1s) and the rest cleared (set to 0s)
    extern Bitboard fileMask[8];

	// an array that contains Bitboards with one rank cleared (set to 0s) and the rest masked (set to 1s)
    extern Bitboard rankClear[8];

	// an array that contains Bitboards with one rank masked (set to 1s) and the rest cleared (set to 0s)
    extern Bitboard rankMask[8];

    void initialize();
    void printBitboard(Bitboard bitboard);
	int getLSB(Bitboard bb);
	int getMSB(Bitboard bb);
}
