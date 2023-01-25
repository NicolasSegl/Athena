#pragma once

#include "Bitboard.h"
#include "DataTypes.h"

// this declaration is necessary to prevent circular including
class Board;

// defines the functions Athena uses for evaluating positions on the board
namespace Eval
{
    // values of the pieces relative to centipawns (1 pawn is worth 100 centipawns)
    const int KING_VALUE   = 20000;
    const int QUEEN_VALUE  = 900;
    const int ROOK_VALUE   = 500;
    const int BISHOP_VALUE = 330;
    const int KNIGHT_VALUE = 320;
    const int PAWN_VALUE   = 100;

    const int CHECKMATE_VALUE = 1000000;

    int evaluateBoardRelativeTo(Colour side, int eval);
    int evaluatePosition(Board* boardPtr, float midgameValue);

    float getMidgameValue(Bitboard occupiedBB);
    int see(Board* boardPtr, Byte square, Colour attackingSide, int currentSquareValue);
    
    void init();
}
