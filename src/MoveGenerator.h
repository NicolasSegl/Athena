#pragma once

#include <vector>

#include "Bitboard.h"
#include "MoveData.h"

class Board;

class MoveGenerator
{
private:
    enum Directions
    {
        DIR_NORTH,
        DIR_SOUTH,
        DIR_EAST,
        DIR_WEST,
        DIR_NORTHEAST,
        DIR_NORTHWEST,
        DIR_SOUTHEAST,
        DIR_SOUTHWEST,
        NUM_DIRECTIONS,
    };

    Bitboard mRays[NUM_DIRECTIONS][64];
    void initRays();

    // initialize nonsliding piece lookup tables
    void initKnightLT(Byte knightLoc);
    void initKingLT(Byte kingLoc);
    void initPawnLT(Colour side, Byte pawnLoc);
    
    // the following functions are used when generating moves
    Bitboard calculatePsuedoMove(Board* board, MoveData* md, Bitboard& pieceBB);

    inline void setEnPassantMoveData(Board* board, int square, Bitboard pieceMovesBB, MoveData* md);
    bool doesCaptureAffectCastle(Board* board, MoveData* md);
    void addMoves(Board* board, Bitboard movesBB, MoveData& md, std::vector<MoveData>& moveVec, bool captureOnly);

    void setCastleMovePrivilegesRevoked(Colour side, Byte privileges, Byte* privilegesToBeRevoked);
    void setCastlePrivileges(Board* board, MoveData* castleMoveData, bool isKing);
    Bitboard* getPieceBitboard(Board* board, Byte square, Colour side = -1); // -1 indicates that no value was passed in
    void getPieceData(Board* boardPtr, Bitboard** pieceBB, Byte* pieceValue, Byte square, Colour side);

public:
    MoveGenerator() {}
    Bitboard pawnAttackLookupTable[2][64]; // 2 because pawns have different moves depending on side
    Bitboard knightLookupTable[64];
    Bitboard kingLookupTable[64];
    
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
    
    void calculateSideMoves(Board* board, Colour side, bool captureOnly = false);
    void calculateCaptureMoves(Board* board, Colour side);
    void calculateCastleMoves(Board* board, Colour side, std::vector<MoveData>& moveVec);
};
