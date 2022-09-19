#include <iostream>

#include "Board.h"
#include "Constants.h"
#include "MoveGeneration.h"

namespace MoveGeneration
{
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

    extern Bitboard pawnAttackLookupTable[2][64] { { 0 }, { 0 } }; // 2 because pawns have different moves depending on side
    extern Bitboard knightLookupTable[64]        { 0 };
    extern Bitboard kingLookupTable[64]          { 0 };

    void initRays()
    {
        for (int fromSquare = 0; fromSquare < 64; fromSquare++)
        {
            for (int dir = 0; dir < NUM_DIRECTIONS; dir++)
                mRays[dir][fromSquare] = 0;

            // cardinal directions
            for (int square = fromSquare + 8; square <= 63; square += 8)
                mRays[DIR_NORTH][fromSquare] |= BB::boardSquares[square];
            for (int square = fromSquare - 8; square >= 0; square -= 8)
                mRays[DIR_SOUTH][fromSquare] |= BB::boardSquares[square];

            // this remainder math is so that the moves stay in the same rank. compare the numbers to https://www.chessprogramming.org/Square_Mapping_Considerations
            for (int square = fromSquare + 1; (square + 1) % 8 != 1 && square <= 63; square++)
                mRays[DIR_EAST][fromSquare] |= BB::boardSquares[square];
            for (int square = fromSquare - 1; (square - 1) % 8 != 6 && square >= 0; square--)
                mRays[DIR_WEST][fromSquare] |= BB::boardSquares[square];

            // ordinal directions
            for (int square = fromSquare + 7; square <= 63 && (square - 1) % 8 != 6; square += 7)
                mRays[DIR_NORTHWEST][fromSquare] |= BB::boardSquares[square];
            for (int square = fromSquare + 9; square <= 63 && (square + 1) % 8 != 1; square += 9)
                mRays[DIR_NORTHEAST][fromSquare] |= BB::boardSquares[square];
            for (int square = fromSquare - 7; square >= 0 && (square + 1) % 8 != 1; square -= 7)
                mRays[DIR_SOUTHWEST][fromSquare] |= BB::boardSquares[square];
            for (int square = fromSquare - 9; square >= 0 && (square - 1) % 8 != 6; square -= 9)
                mRays[DIR_SOUTHEAST][fromSquare] |= BB::boardSquares[square];
        }
    }

    void initKnightLT(Byte knightLoc)
    {
        Bitboard knightAFileClearedBB = BB::boardSquares[knightLoc] & BB::fileClear[BB::FILE_A];
        Bitboard knightBFileClearedBB = BB::boardSquares[knightLoc] & BB::fileClear[BB::FILE_B];
        Bitboard knightGFileClearedBB = BB::boardSquares[knightLoc] & BB::fileClear[BB::FILE_G];
        Bitboard knightHFileClearedBB = BB::boardSquares[knightLoc] & BB::fileClear[BB::FILE_H];

        Bitboard movesBB = 0;

        movesBB |= (knightAFileClearedBB & knightBFileClearedBB) << 6 | (knightAFileClearedBB & knightBFileClearedBB) >> 10; // check western horizontal moves
        movesBB |= knightAFileClearedBB << 15 | knightAFileClearedBB >> 17;                                                  // check western vertical moves
        movesBB |= (knightGFileClearedBB & knightHFileClearedBB) << 10 | (knightGFileClearedBB & knightHFileClearedBB) >> 6;  // check eastern horizontal moves
        movesBB |= knightHFileClearedBB << 17 | knightHFileClearedBB >> 15;                                                  // check eastern vertiacal moves
        knightLookupTable[knightLoc] = movesBB;
    }

    void initKingLT(Byte kingLoc)
    {
        // for moves north west, west, and south west, we need to clear file a to prevent overflow
        Bitboard kingAFileClearedBB = BB::boardSquares[kingLoc] & BB::fileClear[BB::FILE_A];
        // for moves north east, east, and south east, we need to clear file h to prevent overflow
        Bitboard kingHFileClearedBB = BB::boardSquares[kingLoc] & BB::fileClear[BB::FILE_H];

        // consider all ordinal and cardinal directions
        kingLookupTable[kingLoc] = 
                                 BB::northWestOne(kingAFileClearedBB) | BB::eastOne(kingAFileClearedBB) | 
                                 BB::southWestOne(kingAFileClearedBB) | BB::northOne(BB::boardSquares[kingLoc]) |
                                 BB::northEastOne(kingHFileClearedBB) | BB::westOne(kingHFileClearedBB) | 
                                 BB::southEastOne(kingHFileClearedBB) | BB::southOne(BB::boardSquares[kingLoc]);
    }

    void initPawnLT(Colour side, Byte pawnLoc)
    {
        Bitboard pawnAFileClearedBB = BB::boardSquares[pawnLoc] & BB::fileClear[BB::FILE_A];
        Bitboard pawnHFileClearedBB = BB::boardSquares[pawnLoc] & BB::fileClear[BB::FILE_H];

        if (side == SIDE_WHITE)
            pawnAttackLookupTable[SIDE_WHITE][pawnLoc] = BB::northWestOne(pawnAFileClearedBB) | BB::northEastOne(pawnHFileClearedBB);
        else if (side == SIDE_BLACK)
            pawnAttackLookupTable[SIDE_BLACK][pawnLoc] = BB::southWestOne(pawnAFileClearedBB) | BB::southEastOne(pawnHFileClearedBB);
    }

    void init()
    {
        initRays();

        for (int pieceLoc = 0; pieceLoc < 64; pieceLoc++)
        {
            initKingLT(pieceLoc);
            initKnightLT(pieceLoc);
            initPawnLT(SIDE_BLACK, pieceLoc);
            initPawnLT(SIDE_WHITE, pieceLoc);
            //mQueenLookupTable[pieceLoc] = computePseudoRookMoves(pieceLoc, 0, 0) | computePseudoBishopMoves(pieceLoc, 0, 0);
        }
    }

    Bitboard computePseudoKingMoves(Byte fromSquare, Bitboard friendlyPiecesBB)
    {
        // we already have predefined moves for the king so we need only index the correct element of the lookup table
        // then we simply ensure that we aren't allowing moves onto friendly pieces
        return kingLookupTable[fromSquare] & ~friendlyPiecesBB;
    }

    Bitboard computePseudoKnightMoves(Byte fromSquare, Bitboard friendlyPiecesBB)
    {
        // same process as above :D
        return knightLookupTable[fromSquare] & ~friendlyPiecesBB;
    }

    Bitboard computePseudoPawnMoves(Byte fromSquare, Colour side, Bitboard enemyPiecesBB, Bitboard emptyBB, Bitboard enPassantBB)
    {
        // if it's white we need to mask ranks to see if it has permissions to move two squares ahead
        Bitboard movesBB = (pawnAttackLookupTable[side][fromSquare] & enemyPiecesBB) | (pawnAttackLookupTable[side][fromSquare] & enPassantBB);

        if (side == SIDE_WHITE)
        {
            Bitboard oneStepBB = BB::northOne(BB::boardSquares[fromSquare]) & emptyBB;
            // if the twostep is on the fourth rank, it would mean the pawn was on its home row
            Bitboard twoStepBB = ((BB::northOne(oneStepBB)) & BB::rankMask[BB::RANK_FOURTH]) & emptyBB;
            movesBB |= oneStepBB | twoStepBB;
        }
        else if (side == SIDE_BLACK)
        {
            Bitboard oneStepBB = BB::southOne(BB::boardSquares[fromSquare]) & emptyBB;
            // if the twostep is on the fifth rank, it would mean the pawn was on its home row
            Bitboard twoStepBB = ((BB::southOne(oneStepBB)) & BB::rankMask[BB::RANK_FIFTH]) & emptyBB;
            movesBB |= oneStepBB | twoStepBB;
        }

        return movesBB;
    }

    // value of true = stop adding pieces
    inline bool slidingPieceMoveStep(int square, Bitboard* movesBB, Bitboard enemyPiecesBB, Bitboard friendlyPiecesBB)
    {
        if (BB::boardSquares[square] & friendlyPiecesBB)
            return true;

        *movesBB |= BB::boardSquares[square];

        if (BB::boardSquares[square] & enemyPiecesBB)
            return true;

        return false;
    }

    // Uses the classical approach to calculate rook moves. see more here: https://www.chessprogramming.org/Classical_Approach
    Bitboard computePseudoRookMoves(Byte fromSquare, Bitboard occupiedBB, Bitboard friendlyPiecesBB)
    {
        // we have it include enemy and friendly pieces, then at the end we xor any friends pieces out?
        // the operations below temporarily include friendly pieces in the movesBB. these are removed at the end

        if (fromSquare > 64)
            return 0;

        Bitboard movesBB = 0;

        Bitboard currentMoves = mRays[DIR_NORTH][fromSquare];
        Bitboard blockers = currentMoves & occupiedBB;
        if (blockers)
            currentMoves ^= mRays[DIR_NORTH][BB::getLSB(blockers)];
        movesBB |= currentMoves;

        currentMoves = mRays[DIR_SOUTH][fromSquare];
        blockers = currentMoves & occupiedBB;
        if (blockers)
            currentMoves ^= mRays[DIR_SOUTH][BB::getMSB(blockers)];
        movesBB |= currentMoves;

        currentMoves = mRays[DIR_WEST][fromSquare];
        blockers = currentMoves & occupiedBB;
        if (blockers)
            currentMoves ^= mRays[DIR_WEST][BB::getMSB(blockers)];
        movesBB |= currentMoves;

        currentMoves = mRays[DIR_EAST][fromSquare];
        blockers = currentMoves & occupiedBB;
        if (blockers)
            currentMoves ^= mRays[DIR_EAST][BB::getLSB(blockers)];
        movesBB |= currentMoves;

        return movesBB & ~friendlyPiecesBB;
    }

    Bitboard computePseudoBishopMoves(Byte fromSquare, Bitboard occupiedBB, Bitboard friendlyPiecesBB)
    {
        if (fromSquare > 64)
            return 0;

        Bitboard movesBB = 0;

        Bitboard currentMoves = mRays[DIR_NORTHEAST][fromSquare];
        Bitboard blockers = currentMoves & occupiedBB;
        if (blockers)
            currentMoves ^= mRays[DIR_NORTHEAST][BB::getLSB(blockers)];
        movesBB |= currentMoves;

        currentMoves = mRays[DIR_NORTHWEST][fromSquare];
        blockers = currentMoves & occupiedBB;
        if (blockers)
            currentMoves ^= mRays[DIR_NORTHWEST][BB::getLSB(blockers)];
        movesBB |= currentMoves;

        currentMoves = mRays[DIR_SOUTHEAST][fromSquare];
        blockers = currentMoves & occupiedBB;
        if (blockers)
            currentMoves ^= mRays[DIR_SOUTHEAST][BB::getMSB(blockers)];
        movesBB |= currentMoves;

        currentMoves = mRays[DIR_SOUTHWEST][fromSquare];
        blockers = currentMoves & occupiedBB;
        if (blockers)
            currentMoves ^= mRays[DIR_SOUTHWEST][BB::getMSB(blockers)];
        movesBB |= currentMoves;

        return movesBB & ~friendlyPiecesBB;
    }

    Bitboard computePseudoQueenMoves(Byte fromSquare, Bitboard occupiedBB, Bitboard friendlyPiecesBB)
    {
        return computePseudoBishopMoves(fromSquare, occupiedBB, friendlyPiecesBB) | computePseudoRookMoves(fromSquare, occupiedBB, friendlyPiecesBB);
    }

    void setCastleMovePrivilegesRevoked(Colour side, Byte privileges, Byte* privilegesToBeRevoked)
    {
        if (side == SIDE_WHITE)
        {
            if (privileges & (Byte)CastlingPrivilege::WHITE_LONG_CASTLE)  *privilegesToBeRevoked |= (Byte)CastlingPrivilege::WHITE_LONG_CASTLE;
            if (privileges & (Byte)CastlingPrivilege::WHITE_SHORT_CASTLE) *privilegesToBeRevoked |= (Byte)CastlingPrivilege::WHITE_SHORT_CASTLE;
        }
        else
        {
            if (privileges & (Byte)CastlingPrivilege::BLACK_LONG_CASTLE)  *privilegesToBeRevoked |= (Byte)CastlingPrivilege::BLACK_LONG_CASTLE;
            if (privileges & (Byte)CastlingPrivilege::BLACK_SHORT_CASTLE) *privilegesToBeRevoked |= (Byte)CastlingPrivilege::BLACK_SHORT_CASTLE;
        }
    }

    MoveData computeCastleMoveData(Colour side, Byte privileges, Bitboard occupiedBB, CastlingPrivilege castleType)
    {
        MoveData md;
        md.side = side;
        md.setMoveType(MoveData::EncodingBits::REGULAR);
        int lower, higher;

        switch (castleType)
        {
        case CastlingPrivilege::WHITE_SHORT_CASTLE:
        case CastlingPrivilege::BLACK_SHORT_CASTLE:

            if (((privileges & (Byte)CastlingPrivilege::WHITE_SHORT_CASTLE) && side == SIDE_WHITE) ||
                (privileges & (Byte)CastlingPrivilege::BLACK_SHORT_CASTLE) && side == SIDE_BLACK)
            {

                if (side == SIDE_WHITE) { lower = ChessCoord::F1;  higher = ChessCoord::H1; }
                if (side == SIDE_BLACK) { lower = ChessCoord::F8; higher = ChessCoord::H8; }

                // check if the castle would be legal
                for (int tile = lower; tile < higher; tile++)
                    if (BB::boardSquares[tile] & occupiedBB)
                    {
                        md.setMoveType(MoveData::EncodingBits::INVALID);
                        break;
                    }

                if (md.moveType != MoveData::EncodingBits::INVALID)
                {
                    md.setMoveType(MoveData::EncodingBits::SHORT_CASTLE);
                    setCastleMovePrivilegesRevoked(md.side, privileges, &md.castlePrivilegesRevoked);

                    // origin/target square for the move relative to the king
                    md.originSquare = lower - 1;
                    md.targetSquare = higher - 1;
                }

                return md;
            }

            break;

        case CastlingPrivilege::WHITE_LONG_CASTLE:
        case CastlingPrivilege::BLACK_LONG_CASTLE:

            if (((privileges & (Byte)CastlingPrivilege::WHITE_LONG_CASTLE) && side == SIDE_WHITE) ||
                (privileges & (Byte)CastlingPrivilege::BLACK_LONG_CASTLE) && side == SIDE_BLACK)
            {

                if (side == SIDE_WHITE) { lower = ChessCoord::A1;  higher = ChessCoord::D1; }
                if (side == SIDE_BLACK) { lower = ChessCoord::A8; higher = ChessCoord::D8; }

                // check if the castle would be legal
                for (int tile = higher; tile > lower; tile--)
                    if (BB::boardSquares[tile] & occupiedBB)
                    {
                        md.setMoveType(MoveData::EncodingBits::INVALID);
                        break;
                    }
                //md.setMoveType(MoveData::EncodingBits::INVALID);

                if (md.moveType != MoveData::EncodingBits::INVALID)
                {
                    md.setMoveType(MoveData::EncodingBits::LONG_CASTLE);
                    setCastleMovePrivilegesRevoked(md.side, privileges, &md.castlePrivilegesRevoked);

                    // origin/target square for the move relative to the king
                    md.originSquare = higher + 1;
                    md.targetSquare = lower + 2;
                }

                return md;
            }

            break;
        }

        md.setMoveType(MoveData::EncodingBits::INVALID);
        return md;
    }

    void calculateCaptureMoves(Board* board, Colour side, std::vector<MoveData>& moveVec)
    {
        calculateSideMoves(board, side, moveVec, true);
    }

    // side is a default value with a value of -1. this value indicates no side was specified and to search all bitboards
    Bitboard* getPieceBitboard(Board* board, Byte square, Colour side)
    {
        Bitboard squareBB = BB::boardSquares[square];

        if (side == SIDE_WHITE || side == -1)
        {
            if (squareBB & board->currentPosition.whitePawnsBB)        return &board->currentPosition.whitePawnsBB;
            else if (squareBB & board->currentPosition.whiteRooksBB)   return &board->currentPosition.whiteRooksBB;
            else if (squareBB & board->currentPosition.whiteKnightsBB) return &board->currentPosition.whiteKnightsBB;
            else if (squareBB & board->currentPosition.whiteBishopsBB) return &board->currentPosition.whiteBishopsBB;
            else if (squareBB & board->currentPosition.whiteQueensBB)  return &board->currentPosition.whiteQueensBB;
            else if (squareBB & board->currentPosition.whiteKingBB)    return &board->currentPosition.whiteKingBB;
        }
        if (side == SIDE_BLACK || side == -1)
        {
            if (squareBB & board->currentPosition.blackPawnsBB)        return &board->currentPosition.blackPawnsBB;
            else if (squareBB & board->currentPosition.blackRooksBB)   return &board->currentPosition.blackRooksBB;
            else if (squareBB & board->currentPosition.blackKnightsBB) return &board->currentPosition.blackKnightsBB;
            else if (squareBB & board->currentPosition.blackBishopsBB) return &board->currentPosition.blackBishopsBB;
            else if (squareBB & board->currentPosition.blackQueensBB)  return &board->currentPosition.blackQueensBB;
            else if (squareBB & board->currentPosition.blackKingBB)    return &board->currentPosition.blackKingBB;
        }

        return nullptr;
    }

    void getPieceData(Board* boardPtr, Bitboard** pieceBB, Byte* pieceValue, Byte square, Colour side)
    {
        Bitboard squareBB = BB::boardSquares[square];

        if (side == SIDE_WHITE)
        {
            if (squareBB & boardPtr->currentPosition.whitePawnsBB)
            {
                *pieceBB = &boardPtr->currentPosition.whitePawnsBB;
                *pieceValue = 100;
            }
            else if (squareBB & boardPtr->currentPosition.whiteRooksBB)
            {
                *pieceBB = &boardPtr->currentPosition.whiteRooksBB;
                *pieceValue = 500;
            }
            else if (squareBB & boardPtr->currentPosition.whiteKnightsBB)
            {
                *pieceBB = &boardPtr->currentPosition.whiteKnightsBB;
                *pieceValue = 320;
            }
            else if (squareBB & boardPtr->currentPosition.whiteBishopsBB)
            {
                *pieceBB = &boardPtr->currentPosition.whiteBishopsBB;
                *pieceValue = 330;
            }
            else if (squareBB & boardPtr->currentPosition.whiteQueensBB)
            {
                *pieceBB = &boardPtr->currentPosition.whiteQueensBB;
                *pieceValue = 900;
            }
            else if (squareBB & boardPtr->currentPosition.whiteKingBB)
            {
                *pieceBB = &boardPtr->currentPosition.whiteKingBB;
                *pieceValue = 20000;
            }
        }
        else
        {
            if (squareBB & boardPtr->currentPosition.blackPawnsBB)
            {
                *pieceBB = &boardPtr->currentPosition.blackPawnsBB;
                *pieceValue = 100;
            }
            else if (squareBB & boardPtr->currentPosition.blackRooksBB)
            {
                *pieceBB = &boardPtr->currentPosition.blackRooksBB;
                *pieceValue = 500;
            }
            else if (squareBB & boardPtr->currentPosition.blackKnightsBB)
            {
                *pieceBB = &boardPtr->currentPosition.blackKnightsBB;
                *pieceValue = 320;
            }
            else if (squareBB & boardPtr->currentPosition.blackBishopsBB)
            {
                *pieceBB = &boardPtr->currentPosition.blackBishopsBB;
                *pieceValue = 330;
            }
            else if (squareBB & boardPtr->currentPosition.blackQueensBB)
            {
                *pieceBB = &boardPtr->currentPosition.blackQueensBB;
                *pieceValue = 900;
            }
            else if (squareBB & boardPtr->currentPosition.blackKingBB)
            {
                *pieceBB = &boardPtr->currentPosition.blackKingBB;
                *pieceValue = 20000;
            }
        }
    }

    /*






    there could be huge improvements to the finding of castle moves.
    a much more maintanable, readable, and efficient way is possible
    we are calulating it every time, but we should be at least checking the privileges before bothering






    */

    void calculateSideMoves(Board* board, Colour side, std::vector<MoveData>& moveVec, bool captureOnly)
    {
        moveVec.clear();
        moveVec.reserve(64);
        Bitboard colourBB = side == SIDE_WHITE ? board->currentPosition.whitePiecesBB : board->currentPosition.blackPiecesBB;

        for (int square = 0; square < 64; square++)
            if (BB::boardSquares[square] & colourBB)
                calculatePieceMoves(board, side, square, moveVec, captureOnly);

        // could we remove this and only calculate castle moves when we come across a king?
        if (!captureOnly)
            calculateCastleMoves(board, side, moveVec);
    }

    void calculateCastleMoves(Board* board, Colour side, std::vector<MoveData>& movesVec)
    {
        // check if castle moves are possible before computing them (otherwise resulting in invalid moves)
        MoveData shortCastleMD, longCastleMD;

        if (side == SIDE_WHITE) // white castle moves
        {
            if (board->currentPosition.whiteKingBB == 0 || board->currentPosition.whiteRooksBB == 0)
                return;

            shortCastleMD = computeCastleMoveData(side, board->currentPosition.castlePrivileges, board->currentPosition.occupiedBB, CastlingPrivilege::WHITE_SHORT_CASTLE);
            longCastleMD  = computeCastleMoveData(side, board->currentPosition.castlePrivileges, board->currentPosition.occupiedBB, CastlingPrivilege::WHITE_LONG_CASTLE);
        }
        else // black castle moves
        {
            if (board->currentPosition.blackKingBB == 0 || board->currentPosition.blackRooksBB == 0)
                return;

            shortCastleMD = computeCastleMoveData(side, board->currentPosition.castlePrivileges, board->currentPosition.occupiedBB, CastlingPrivilege::BLACK_SHORT_CASTLE);
            longCastleMD  = computeCastleMoveData(side, board->currentPosition.castlePrivileges, board->currentPosition.occupiedBB, CastlingPrivilege::BLACK_LONG_CASTLE);
        }

        if (shortCastleMD.moveType != MoveData::EncodingBits::INVALID)
            movesVec.push_back(shortCastleMD);
        if (longCastleMD.moveType != MoveData::EncodingBits::INVALID)
            movesVec.push_back(longCastleMD);
    }

    bool doesCaptureAffectCastle(Board* board, MoveData* md)
    {
        bool doesAffectCastlePrivileges = false;
        if (md->capturedPieceBB == &board->currentPosition.blackRooksBB)
        {
            doesAffectCastlePrivileges = true;
            if (md->targetSquare == ChessCoord::H8 && (board->currentPosition.castlePrivileges & (Byte)CastlingPrivilege::BLACK_SHORT_CASTLE))
                md->castlePrivilegesRevoked |= (Byte)CastlingPrivilege::BLACK_SHORT_CASTLE;
            else if (md->targetSquare == ChessCoord::A1 && (board->currentPosition.castlePrivileges & (Byte)CastlingPrivilege::BLACK_LONG_CASTLE))
                md->castlePrivilegesRevoked |= (Byte)CastlingPrivilege::BLACK_LONG_CASTLE;
            else
                doesAffectCastlePrivileges = false;
        }
        else if (md->capturedPieceBB == &board->currentPosition.whiteRooksBB)
        {
            doesAffectCastlePrivileges = true;
            if (md->targetSquare == ChessCoord::H1 && (board->currentPosition.castlePrivileges & (Byte)CastlingPrivilege::WHITE_SHORT_CASTLE))
                md->castlePrivilegesRevoked |= (Byte)CastlingPrivilege::WHITE_SHORT_CASTLE;
            else if (md->targetSquare == ChessCoord::A1 && (board->currentPosition.castlePrivileges & (Byte)CastlingPrivilege::WHITE_LONG_CASTLE))
                md->castlePrivilegesRevoked |= (Byte)CastlingPrivilege::WHITE_LONG_CASTLE;
            else
                doesAffectCastlePrivileges = false;
        }

        return doesAffectCastlePrivileges;
    }

    inline void setEnPassantMoveData(Board* board, int square, Bitboard pieceMovesBB, MoveData* md)
    {
        if (board->currentPosition.enPassantSquare < 64)
        {
            if (BB::boardSquares[square] & BB::boardSquares[board->currentPosition.enPassantSquare])
            {
                if (pieceMovesBB & BB::boardSquares[board->currentPosition.enPassantSquare])
                {
                    if (md->pieceBB == &board->currentPosition.whitePawnsBB)
                    {
                        md->capturedPieceBB = getPieceBitboard(board, square - 8, SIDE_BLACK);
                        md->setMoveType(MoveData::EncodingBits::EN_PASSANT_CAPTURE);
                        md->enPassantSquare = board->currentPosition.enPassantSquare;
                    }
                    else if (md->pieceBB == &board->currentPosition.blackPawnsBB)
                    {
                        md->capturedPieceBB = getPieceBitboard(board, square + 8, SIDE_WHITE);
                        md->setMoveType(MoveData::EncodingBits::EN_PASSANT_CAPTURE);
                        md->enPassantSquare = board->currentPosition.enPassantSquare;
                    }
                }
            }

            md->enPassantSquare = board->currentPosition.enPassantSquare;
        }
    }

    void addMoves(Board* board, Bitboard movesBB, MoveData& md, std::vector<MoveData>& moveVec, bool captureOnly)
    {
        for (int square = 0; square < 64; square++)
        {
            if (movesBB & BB::boardSquares[square])
            {
                md.targetSquare = square;
                md.capturedPieceBB = nullptr;
                md.setMoveType(MoveData::EncodingBits::REGULAR);

                if (BB::boardSquares[square] & *md.capturedColourBB)
                    getPieceData(board, &md.capturedPieceBB, &md.capturedPieceValue, square, !md.side);

                if (captureOnly && !md.capturedPieceBB)
                    continue;

                if (md.pieceBB == &board->currentPosition.whitePawnsBB && md.targetSquare >= ChessCoord::A8)  md.setMoveType(MoveData::EncodingBits::PAWN_PROMOTION);
                if (md.pieceBB == &board->currentPosition.blackPawnsBB && md.targetSquare <= ChessCoord::H1)  md.setMoveType(MoveData::EncodingBits::PAWN_PROMOTION);
                setEnPassantMoveData(board, square, movesBB, &md);

                // we need to reset the privileges revoked after a move that captures a rook has changed them
                // otherwise, no matter what move the piece makes (i.e. no capturing the rook), will change the privileges
                bool resetCastlePrivileges = doesCaptureAffectCastle(board, &md);

                moveVec.push_back(md);

                if (resetCastlePrivileges)
                    md.castlePrivilegesRevoked = 0;
            }
        }
    }

    void setCastlePrivileges(Board* board, MoveData* md, bool isKing)
    {
        if (isKing)
        {
            if (md->side == SIDE_WHITE && md->originSquare == ChessCoord::E1 && (board->currentPosition.castlePrivileges & (Byte)CastlingPrivilege::WHITE_LONG_CASTLE))
                md->castlePrivilegesRevoked |= (Byte)CastlingPrivilege::WHITE_LONG_CASTLE;
            if (md->side == SIDE_WHITE && md->originSquare == ChessCoord::E1 && (board->currentPosition.castlePrivileges & (Byte)CastlingPrivilege::WHITE_SHORT_CASTLE))
                md->castlePrivilegesRevoked |= (Byte)CastlingPrivilege::WHITE_SHORT_CASTLE;
            if (md->side == SIDE_BLACK && md->originSquare == ChessCoord::E8 && (board->currentPosition.castlePrivileges & (Byte)CastlingPrivilege::BLACK_LONG_CASTLE))
                md->castlePrivilegesRevoked |= (Byte)CastlingPrivilege::BLACK_LONG_CASTLE;
            if (md->side == SIDE_BLACK && md->originSquare == ChessCoord::E8 && (board->currentPosition.castlePrivileges & (Byte)CastlingPrivilege::BLACK_SHORT_CASTLE))
                md->castlePrivilegesRevoked |= (Byte)CastlingPrivilege::BLACK_SHORT_CASTLE;
        }
        else
        {
            if (md->side == SIDE_WHITE && md->originSquare == ChessCoord::A1 && (board->currentPosition.castlePrivileges & (Byte)CastlingPrivilege::WHITE_LONG_CASTLE))
                md->castlePrivilegesRevoked |= (Byte)CastlingPrivilege::WHITE_LONG_CASTLE;
            else if (md->side == SIDE_WHITE && md->originSquare == ChessCoord::H1 && (board->currentPosition.castlePrivileges & (Byte)CastlingPrivilege::WHITE_SHORT_CASTLE))
                md->castlePrivilegesRevoked |= (Byte)CastlingPrivilege::WHITE_SHORT_CASTLE;
            else if (md->side == SIDE_BLACK && md->originSquare == ChessCoord::A8 && (board->currentPosition.castlePrivileges & (Byte)CastlingPrivilege::BLACK_LONG_CASTLE))
                md->castlePrivilegesRevoked |= (Byte)CastlingPrivilege::BLACK_LONG_CASTLE;
            else if (md->side == SIDE_BLACK && md->originSquare == ChessCoord::H8 && (board->currentPosition.castlePrivileges & (Byte)CastlingPrivilege::BLACK_SHORT_CASTLE))
                md->castlePrivilegesRevoked |= (Byte)CastlingPrivilege::BLACK_SHORT_CASTLE;
        }
    }

    Bitboard calculatePsuedoMove(Board* board, MoveData* md, Bitboard& pieceBB)
    {
        if ((pieceBB & board->currentPosition.whiteKnightsBB) || (pieceBB & board->currentPosition.blackKnightsBB))
            return computePseudoKnightMoves(md->originSquare, *md->colourBB);

        else if ((pieceBB & board->currentPosition.whitePawnsBB) || (pieceBB & board->currentPosition.blackPawnsBB))
            return computePseudoPawnMoves(md->originSquare, md->side, *md->capturedColourBB, board->currentPosition.emptyBB, BB::boardSquares[board->currentPosition.enPassantSquare]);

        else if ((pieceBB & board->currentPosition.whiteBishopsBB) || (pieceBB & board->currentPosition.blackBishopsBB))
            return computePseudoBishopMoves(md->originSquare, board->currentPosition.occupiedBB, *md->colourBB);

        else if ((pieceBB & board->currentPosition.whiteQueensBB) || (pieceBB & board->currentPosition.blackQueensBB))
            return computePseudoQueenMoves(md->originSquare, board->currentPosition.occupiedBB, *md->colourBB);

        else if ((pieceBB & board->currentPosition.whiteRooksBB) || (pieceBB & board->currentPosition.blackRooksBB)) // these moves may change castling privileges
        {
            setCastlePrivileges(board, md, false);
            return computePseudoRookMoves(md->originSquare, board->currentPosition.occupiedBB, *md->colourBB);
        }

        else if ((pieceBB & board->currentPosition.whiteKingBB) || (pieceBB & board->currentPosition.blackKingBB)) // these moves may change castling privileges
        {
            setCastlePrivileges(board, md, true);
            return computePseudoKingMoves(md->originSquare, *md->colourBB);
        }
    }

    void calculatePieceMoves(Board* board, Colour side, Byte originSquare, std::vector<MoveData>& moveVec, bool captureOnly)
    {
        MoveData md;
        md.colourBB = side == SIDE_WHITE ? &board->currentPosition.whitePiecesBB : &board->currentPosition.blackPiecesBB;
        md.capturedColourBB = side == SIDE_WHITE ? &board->currentPosition.blackPiecesBB : &board->currentPosition.whitePiecesBB;

        md.side = side;
        md.originSquare = originSquare;

        Bitboard movesBB = 0;
        getPieceData(board, &md.pieceBB, &md.pieceValue, originSquare, side);

        movesBB = calculatePsuedoMove(board, &md, *md.pieceBB);

        if (movesBB > 0)
            addMoves(board, movesBB, md, moveVec, captureOnly);
    }
}