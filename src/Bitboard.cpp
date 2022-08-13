#include "Bitboard.h"

#include <assert.h>
#include <iostream>

namespace BB
{
    extern Bitboard boardSquares[64]	          { 0 };
    extern Bitboard fileClear[8]		          { 0 };
    extern Bitboard rankClear[8]		          { 0 };

    void initializeFileRankMasks()
    {
        for (int i = 0; i < 8; i++)
        {
            // intentional overflow making fileClear[i]'s and rankClear[i]'s bits all set to 1
            fileClear[i] = (Bitboard)(-1);
            rankClear[i] = (Bitboard)(-1);

            for (int j = 0; j < 8; j++)
            {
                fileClear[i] ^= (Bitboard)1 << j * 8 + i;
                rankClear[i] ^= (Bitboard)1 << i * 8 + j;
            }
        }
    }

    void printBitboard(Bitboard bitboard)
    {
        // have to print by last rank first,
        // but print files left to right

        int cRank = 8;
        int cFile = 0;

        while (cRank > 0)
        {
            if (bitboard & (boardSquares[(cRank - 1) * 8 + cFile]))
                std::cout << 1;
            else
                std::cout << 0;

            cFile++;

            if (cFile >= 8)
            {
                std::cout << std::endl;
                cFile = 0;
                cRank--;
            }
        }
        std::cout << std::endl;
    }

    void initialize()
    {
        initializeFileRankMasks();
        
        for (int square = 0; square < 64; square++)
            boardSquares[square] = (Bitboard)1 << square;
    }

    /*
    the following code for calculating most/least significant bits is taken from https://www.chessprogramming.org/BitScan#De_Bruijn_Multiplication
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

    int getLSB(Bitboard bb)
    {
        assert(bb != 0);
        const uint64_t debruijn64 = uint64_t(0x03f79d71b4cb0a89);
        return index64[((bb ^ (bb - 1)) * debruijn64) >> 58];
    }

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
