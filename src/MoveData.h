#pragma once

#include "DataTypes.h"

// this enum defines the bit encoding for castle privileges (convinient in properly setting or unsetting castling 
// privileges when making/unmaking moves)
enum class CastlingPrivilege
{
    WHITE_LONG_CASTLE  = 1 << 0,
    WHITE_SHORT_CASTLE = 1 << 1,
    BLACK_LONG_CASTLE  = 1 << 2,
    BLACK_SHORT_CASTLE = 1 << 3,
};

// this structure contains all of the information for the engine's abstractions to fully make a move and unmake the same move
// when the engine generates all of the possible moves, an array of type MoveData is returned
struct MoveData
{
    // this enum defines the possible types of moves the move can be
    // it is used for the sake of making/unmaking moves, as some moves can require special handling
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
        CASTLE_HALF_MOVE,
    };

    // stores the type of move that the move was (with values according to the enum above)
    EncodingBits moveType = EncodingBits::INVALID;

    /* pieceBB and colourBB must be pointers for the sake of incremental updating */

    // a pointer to the Bitboard containing all of the same colour and type of piece as the piece moving 
    Bitboard* pieceBB  = nullptr;

    // a pointer to the Bitboard containing all of the same colour of pieces as the piece moving
    Bitboard* colourBB = nullptr;

    // a pointer to the Bitboard containing all of the same colour and type of piece as the piece that was captured 
    // if this variable is nullptr, it indicates that the move does not involve a capture
    Bitboard* capturedPieceBB = nullptr;

    // a pointer to the Bitboard containing all of the same colour of pieces as the piece that was captured
    Bitboard* capturedColourBB = nullptr;

    // gets the integral coordinate of the square that has an en passant set on it (0-63)
    // an en passant square value of >63 indicates that there are no en passant squares
    Byte enPassantSquare = 100;

    // stores which side is able to make the move
    Colour side;

    // contains the integral coordinate of where the piece was origianlly located (0-63)
    Byte originSquare;

    // contains the integral coordinate of where the piece went to (0-63)
    Byte targetSquare;

    // stores the value of the piece that moved
    int pieceValue;

    // stores the value of the piece that was captured
    int capturedPieceValue;

    // if any castle privileges were taken away during the move, the appropriate bits will be set in this variable as defined above 
    // in the CastlePrivileges enum
    Byte castlePrivilegesRevoked = 0;

    // stores the weighting of the move in regards to prioritization. that is, this variable describes how early we should try to search
    // this move. a high value indicates more urgency
    int moveScore = 0;

    // contains the number of full-moves since the last pawn move or piece capture 
    Byte fiftyMoveCounter;

    // a legacy function used early on in development. simply sets the move type of the move
    void setMoveType(EncodingBits mt) { moveType = mt; }

    // compare two moves to see if they are the same 
    bool operator==(MoveData& rightMD)
    {
        return originSquare == rightMD.originSquare && targetSquare == rightMD.targetSquare &&
               pieceBB == rightMD.pieceBB && colourBB == rightMD.colourBB &&
               capturedPieceBB == rightMD.capturedPieceBB && enPassantSquare == rightMD.enPassantSquare &&
               castlePrivilegesRevoked == rightMD.castlePrivilegesRevoked && moveType == rightMD.moveType;
    }
};