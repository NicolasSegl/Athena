#pragma once

#include "DataTypes.h"

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
        CAPTURE,
        LONG_CASTLE,
        SHORT_CASTLE,
        INVALID,
        REGULAR,
        EN_PASSANT_CAPTURE,
        QUEEN_PROMO,
        ROOK_PROMO,
        BISHOP_PROMO,
        KNIGHT_PROMO,
        EN_PASSANT_SQUARE,
        PAWN_PROMOTION,
        CASTLE_HALF_MOVE, // for king and rook moves made for the sake of castling
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
    Byte pieceValue;
    Byte capturedPieceValue;

    // castling rights, en passant, half-move counter... etc https://www.chessprogramming.org/Encoding_Moves
    EncodingBits moveType = EncodingBits::INVALID;
    Byte castlePrivilegesRevoked = 0;
    int moveScore = 0;
    Byte fiftyMoveCounter;

    void setMoveType(EncodingBits mt) { moveType = mt; }

    bool operator==(MoveData& rightMD)
    {
        return originSquare == rightMD.originSquare && targetSquare == rightMD.targetSquare &&
               pieceBB == rightMD.pieceBB && colourBB == rightMD.colourBB &&
               capturedPieceBB == rightMD.capturedPieceBB && enPassantSquare == rightMD.enPassantSquare &&
               castlePrivilegesRevoked == rightMD.castlePrivilegesRevoked && moveType == rightMD.moveType;
    }
};