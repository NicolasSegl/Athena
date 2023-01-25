#include <assert.h>
#include <iostream>

#include "Bitboard.h"

namespace BB
{
    /* initialize all of the global variables defined in Bitboard.h so that they may be used elsewhere by the engine */
    
    extern Bitboard adjacentFiles[8] { 0 };
	extern Bitboard eastFile[8]      { 0 };
	extern Bitboard westFile[8]      { 0 };

    extern Bitboard boardSquares[64] { 0 };

    extern Bitboard fileClear[8]     { 0 };
    extern Bitboard fileMask[8]      { 0 };

    extern Bitboard rankClear[8]     { 0 };
    extern Bitboard rankMask[8]      { 0 };

    // initializes the arrays that mask or clear entire rows and ranks
    void initFileRankMasks()
    {
        // i representing the rank/file (A-H or 1-8)
        for (int i = 0; i < 8; i++)
        {
            // intentional overflow making fileClear[i]'s and rankClear[i]'s bits all set to 1
            fileClear[i] = (Bitboard)(-1);
            rankClear[i] = (Bitboard)(-1);

            // j represeneting the file for rankClear and rankMask, while also representing the rank for fileClear and fileMask
            for (int j = 0; j < 8; j++)
            {
                // reset an entire file's bits to 0
                fileClear[i] ^= (Bitboard)1 << j * 8 + i;

                // set an entire file's bits to 1
                fileMask[i]  |= (Bitboard)1 << j * 8 + i;
                
                // reset an entire rank's bits to 0
                rankClear[i] ^= (Bitboard)1 << i * 8 + j;

                // set an entire rank's bits to 1
                rankMask[i]  |= (Bitboard)1 << i * 8 + j;
            }
        }
    }

    // initalizes the west, east, and adjacent files arrays. a description of what the arrays do can be found at their declaration in Bitboard.h
    void initAdjacentFilesBitboards()
    {
        for (int i = 0; i < 8; i++)
        {
            // the below if statements ensure that the westFile array has no file set for the file to the west of file A,
            // and that the eastFile array has no file set for the file to the east of file H 
            if (i > 0) westFile[i] |= fileMask[i - 1];
            if (i < 7) eastFile[i] |= fileMask[i + 1];

            // each element in the adjacentFiles array is just the Bitboard with 1s set for the files to the east/west of the index
            adjacentFiles[i] |= westFile[i] | eastFile[i];
        }
    }

    // uses the standard output to print the bitboard provided as the arguement
    void printBitboard(Bitboard bitboard)
    {
        // cRank is 8 because we have to print by the last rank first,
        int cRank = 8;

        // cFile is 0 because we have to print the leftmost file first
        int cFile = 0;

        // when cRank is 0, we will have gotten down and drawn the first rank (and so will be finished printing the bitboard to the console)
        while (cRank > 0)
        {
            // print a 1 if the bit in the bitboard is set, and a 0 otherwise
            if (bitboard & (boardSquares[(cRank - 1) * 8 + cFile]))
                std::cout << 1;
            else
                std::cout << 0;

            // every 8 squares, print a newline character
            cFile++;
            if (cFile >= 8)
            {
                std::cout << std::endl;

                // reset to the leftmost file after an entire rank has been printed
                cFile = 0;

                // decrement cRank until we reach the 1st rank
                cRank--;
            }
        }

        // cout a newline after printing the entire bitboard
        std::cout << std::endl;
    }

    // initializes all of the general purpose bitboards that Athena will be using
    void initialize()
    {
        initFileRankMasks();
        initAdjacentFilesBitboards();
        
        // set exactly one bit in each Bitboard of the array that represents each individual square
        for (int square = 0; square < 64; square++)
            boardSquares[square] = (Bitboard)1 << square;
    }

    /*
        the following code for calculating most and least significant bits is taken from https://www.chessprogramming.org/BitScan#De_Bruijn_Multiplication
    */
    const int index64[64] = {
        0, 47,  1, 56, 48, 27,  2, 60,
       57, 49, 41, 37, 28, 16,  3, 61,
       54, 58, 35, 52, 50, 42, 21, 44,
       38, 32, 29, 23, 17, 11,  4, 62,
       46, 55, 26, 59, 40, 36, 15, 53,
       34, 51, 20, 43, 31, 22, 10, 45,
       25, 39, 14, 33, 19, 30,  9, 24,
       13, 18,  8, 12,  7,  6,  5, 63
    };

    // returns the last significant bit in the Bitboard
    int getLSB(Bitboard bb)
    {
        assert(bb != 0);
        const uint64_t debruijn64 = uint64_t(0x03f79d71b4cb0a89);
        return index64[((bb ^ (bb - 1)) * debruijn64) >> 58];
    }

    // ret urns the most significant bit in the Bitboard
    int getMSB(Bitboard bb)
    {
        assert(bb != 0);
        const uint64_t debruijn64 = uint64_t(0x03f79d71b4cb0a89);
        bb |= bb >> 1;
        bb |= bb >> 2;
        bb |= bb >> 4;
        bb |= bb >> 8;
        bb |= bb >> 16;
        bb |= bb >> 32;
        return index64[(bb * debruijn64) >> 58];
    }
}
