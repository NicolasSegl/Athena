#pragma once

#include <vector>

#include "Bitboard.h"
#include "MoveData.h"

// this declaration is necessary to prevent circular including
class Board;

// defines all of the things used for the generation of moves
namespace MoveGeneration
{   
    /* 
        non sliding pieces do not have their moves dictated by the positions of other pieces on the board, 
        and thus previously calculated lookup tables for their moves can be fully computed prior to move
        generation
    */

    // each element is a Bitboard with all the possible moves for pawns (assuming no pieces are blocking)
    // it's a 2d array as pawns on opposite sides move in opposite directions
    extern Bitboard pawnAttackLookupTable[2][64]; 

    // each element is a Bitboard with all the possible moves for knights
    extern Bitboard knightLookupTable[64];

    // each element is a Bitboard with all the possible moves for the king (assuming no pieces are blocking)
    extern Bitboard kingLookupTable[64];

    void init();

    /* pseudo meaning that they do not account for checks */

    Bitboard computePseudoKingMoves(Byte fromSquare, Bitboard friendlyPiecesBB);
    Bitboard computePseudoKnightMoves(Byte fromSquare, Bitboard friendlyPiecesBB);
    Bitboard computePseudoPawnMoves(Byte fromSquare, Colour side, Bitboard enemyPiecesBB, Bitboard occupiedSquaresBB, Byte enPassantSquare);
    Bitboard computePseudoRookMoves(Byte fromSquare, Bitboard occupiedBB, Bitboard friendlyPiecesBB);
    Bitboard computePseudoBishopMoves(Byte fromSquare, Bitboard occupiedBB, Bitboard friendlyPiecesBB);
    Bitboard computePseudoQueenMoves(Byte fromSquare, Bitboard occupiedBB, Bitboard friendlyPiecesBB);

    void calculatePieceMoves(Board* board, Colour side, Byte originSquare, std::vector<MoveData>& moveVec, bool captureOnly);
    MoveData computeCastleMoveData(Colour side, Byte privileges, Bitboard occupiedBB, CastlingPrivilege castleType);
    
    void calculateSideMoves(Board* board, Colour side, std::vector<MoveData>& moveVec, bool captureOnly = false);
    void calculateCaptureMoves(Board* board, Colour side, std::vector<MoveData>& moveVec);
    void calculateCastleMoves(Board* board, Colour side, std::vector<MoveData>& moveVec);
};
