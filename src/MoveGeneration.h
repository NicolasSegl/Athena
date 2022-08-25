#pragma once

#include <vector>

#include "Bitboard.h"
#include "MoveData.h"

class Board;

namespace MoveGeneration
{   
    extern Bitboard pawnAttackLookupTable[2][64]; // 2 because pawns have different moves depending on side
    extern Bitboard knightLookupTable[64];
    extern Bitboard kingLookupTable[64];

    void init();

    // pseudo meaning that they do not account for if they result in a check!
    Bitboard computePseudoKingMoves(Byte fromSquare, Bitboard friendlyPiecesBB);
    Bitboard computePseudoKnightMoves(Byte fromSquare, Bitboard friendlyPiecesBB);
    Bitboard computePseudoPawnMoves(Byte fromSquare, Colour side, Bitboard enemyPiecesBB, Bitboard occupiedSquaresBB, Bitboard enPassantBB);
    Bitboard computePseudoRookMoves(Byte fromSquare, Bitboard occupiedBB, Bitboard friendlyPiecesBB);
    Bitboard computePseudoBishopMoves(Byte fromSquare, Bitboard occupiedBB, Bitboard friendlyPiecesBB);
    Bitboard computePseudoQueenMoves(Byte fromSquare, Bitboard occupiedBB, Bitboard friendlyPiecesBB);

    void calculatePieceMoves(Board* board, Colour side, Byte originSquare, std::vector<MoveData>& moveVec, bool captureOnly);
    MoveData computeCastleMoveData(Colour side, Byte privileges, Bitboard occupiedBB, CastlingPrivilege castleType);
    
    void calculateSideMoves(Board* board, Colour side, std::vector<MoveData>& moveVec, bool captureOnly = false);
    void calculateCaptureMoves(Board* board, Colour side, std::vector<MoveData>& moveVec);
    void calculateCastleMoves(Board* board, Colour side, std::vector<MoveData>& moveVec);
};
