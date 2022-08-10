#include "Bitboard.h"

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
}
