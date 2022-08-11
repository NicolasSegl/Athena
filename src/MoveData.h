#ifndef MoveData_hpp
#define MoveData_hpp

#include "Constants.h"

enum class CastlingPrivilege
{
    WHITE_LONG_CASTLE  = 1 << 0,
    WHITE_SHORT_CASTLE = 1 << 1,
    BLACK_LONG_CASTLE  = 1 << 2,
    BLACK_SHORT_CASTLE = 1 << 3,
};

struct MoveData
{
    // insofar, capture and en_passant_square are never used. unsure about invalid/none's purposes
    enum class EncodingBits
    {
        CAPTURE = 1 << 0,
        LONG_CASTLE = 1 << 1,
        SHORT_CASTLE = 1 << 2,
        INVALID = 1 << 3,
        NONE = 1 << 4,
        REGULAR = 1 << 5,
        EN_PASSANT_CAPTURE = 1 << 6,
        QUEEN_PROMO = 1 << 7,
        ROOK_PROMO = 1 << 8,
        BISHOP_PROMO = 1 << 9,
        KNIGHT_PROMO = 1 << 10,
        EN_PASSANT_SQUARE = 1 << 11,
        PAWN_PROMOTION = 1 << 12,
        CASTLE_HALF_MOVE = 1 << 13, // for king and rook moves made for the sake of castling
    };

    // these must be pointers for the sake of incremental updating
    Bitboard* pieceBB  = nullptr;
    Bitboard* colourBB = nullptr;

    // if these are not null pointers then a capture has taken place
    Bitboard* capturedPieceBB = nullptr;
    Bitboard* capturedColourBB = nullptr;

    // an en passant square value of >64 indicates that there are no en passant squares
    Byte enPassantSquare = 100;

    Colour side;

    // only a byte long because they are only 0-63
    Byte originSquare;
    Byte targetSquare;

    // castling rights, en passant, half-move counter... etc https://www.chessprogramming.org/Encoding_Moves
    EncodingBits moveType = EncodingBits::INVALID;
    Byte castlePrivilegesRevoked = 0;
    Byte moveScore = 0;
    int fiftyMoveCounter;

    void setMoveType(EncodingBits mt) { moveType = mt; }

    bool operator==(MoveData& rightMD)
    {
        return originSquare == rightMD.originSquare && targetSquare == rightMD.targetSquare &&
               pieceBB == rightMD.pieceBB && colourBB == rightMD.colourBB &&
               capturedPieceBB == rightMD.capturedPieceBB && enPassantSquare == rightMD.enPassantSquare &&
               castlePrivilegesRevoked == rightMD.castlePrivilegesRevoked && moveType == rightMD.moveType;
    }
};

#endif /* MoveData_hpp */
