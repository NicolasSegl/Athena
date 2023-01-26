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

    // each element consists of an array of 64 Bitboards, each of which masks all the bits in one direction starting from a certain square
    Bitboard rays[NUM_DIRECTIONS][64];

    /* initialize all of the global variables defined in MoveGeneration.h so that they may be used elsewhere by the engine */

    extern Bitboard pawnAttackLookupTable[2][64] { { 0 }, { 0 } };
    extern Bitboard knightLookupTable[64]        { 0 };
    extern Bitboard kingLookupTable[64]          { 0 };

    // initializes the rays table
    void initRays()
    {
        for (int fromSquare = 0; fromSquare < 64; fromSquare++)
        {
            // clear all the bits of the Bitboard before setting any
            for (int dir = 0; dir < NUM_DIRECTIONS; dir++)
                rays[dir][fromSquare] = 0;

            // set the bits for the cardinal directions
            for (int square = fromSquare + 8; square <= 63; square += 8)
                rays[DIR_NORTH][fromSquare] |= BB::boardSquares[square];
            for (int square = fromSquare - 8; square >= 0; square -= 8)
                rays[DIR_SOUTH][fromSquare] |= BB::boardSquares[square];

            // this remainder math is so that the moves stay in the same rank (i.e., do not overflow)
            // here's a reference https://www.chessprogramming.org/Square_Mapping_Considerations
            for (int square = fromSquare + 1; (square + 1) % 8 != 1 && square <= 63; square++)
                rays[DIR_EAST][fromSquare] |= BB::boardSquares[square];
            for (int square = fromSquare - 1; (square - 1) % 8 != 6 && square >= 0; square--)
                rays[DIR_WEST][fromSquare] |= BB::boardSquares[square];

            // set bits for the ordinal directions
            for (int square = fromSquare + 7; square <= 63 && (square - 1) % 8 != 6; square += 7)
                rays[DIR_NORTHWEST][fromSquare] |= BB::boardSquares[square];
            for (int square = fromSquare + 9; square <= 63 && (square + 1) % 8 != 1; square += 9)
                rays[DIR_NORTHEAST][fromSquare] |= BB::boardSquares[square];
            for (int square = fromSquare - 7; square >= 0 && (square + 1) % 8 != 1; square -= 7)
                rays[DIR_SOUTHWEST][fromSquare] |= BB::boardSquares[square];
            for (int square = fromSquare - 9; square >= 0 && (square - 1) % 8 != 6; square -= 9)
                rays[DIR_SOUTHEAST][fromSquare] |= BB::boardSquares[square];
        }
    }

    // initializes the knight pseudomove lookup table
    void initKnightLT(Byte knightLoc)
    {
        // these bitboards clear a certain file as to prevent bits overflowwing into ranks they shouldn't
        Bitboard knightAFileClearedBB = BB::boardSquares[knightLoc] & BB::fileClear[BB::FILE_A];
        Bitboard knightBFileClearedBB = BB::boardSquares[knightLoc] & BB::fileClear[BB::FILE_B];
        Bitboard knightGFileClearedBB = BB::boardSquares[knightLoc] & BB::fileClear[BB::FILE_G];
        Bitboard knightHFileClearedBB = BB::boardSquares[knightLoc] & BB::fileClear[BB::FILE_H];

        Bitboard movesBB = 0;

        // add western horizontal moves
        movesBB |= (knightAFileClearedBB & knightBFileClearedBB) << 6 | (knightAFileClearedBB & knightBFileClearedBB) >> 10;

        // add western vertical moves
        movesBB |= knightAFileClearedBB << 15 | knightAFileClearedBB >> 17;

        // add eastern horizontal moves                                                 
        movesBB |= (knightGFileClearedBB & knightHFileClearedBB) << 10 | (knightGFileClearedBB & knightHFileClearedBB) >> 6;

        // add eastern vertical moves
        movesBB |= knightHFileClearedBB << 17 | knightHFileClearedBB >> 15;                         

        knightLookupTable[knightLoc] = movesBB;
    }

    // initializes the king's pseudomove lookup table
    void initKingLT(Byte kingLoc)
    {
        // for moves north west, west, and south west, we need to clear file a to prevent overflow
        Bitboard kingAFileClearedBB = BB::boardSquares[kingLoc] & BB::fileClear[BB::FILE_A];

        // for moves north east, east, and south east, we need to clear file h to prevent overflow
        Bitboard kingHFileClearedBB = BB::boardSquares[kingLoc] & BB::fileClear[BB::FILE_H];

        // consider all ordinal and cardinal directions
        kingLookupTable[kingLoc] = BB::northWestOne(kingAFileClearedBB) | BB::eastOne(kingAFileClearedBB) | 
                                   BB::southWestOne(kingAFileClearedBB) | BB::northOne(BB::boardSquares[kingLoc]) |
                                   BB::northEastOne(kingHFileClearedBB) | BB::westOne(kingHFileClearedBB) | 
                                   BB::southEastOne(kingHFileClearedBB) | BB::southOne(BB::boardSquares[kingLoc]);
    }

    // initializes the pawn pseudomove lookup table
    void initPawnLT(Colour side, Byte pawnLoc)
    {
        // clears the A file to prevent diagonal moves to the west causing overflow into another rank
        Bitboard pawnAFileClearedBB = BB::boardSquares[pawnLoc] & BB::fileClear[BB::FILE_A];

        // clears the H file to prevent diagonal moves to the east causing overflow into another rank
        Bitboard pawnHFileClearedBB = BB::boardSquares[pawnLoc] & BB::fileClear[BB::FILE_H];

        // set the diagonal attacks for the pawns
        if (side == SIDE_WHITE)
            pawnAttackLookupTable[SIDE_WHITE][pawnLoc] = BB::northWestOne(pawnAFileClearedBB) | BB::northEastOne(pawnHFileClearedBB);
        else if (side == SIDE_BLACK)
            pawnAttackLookupTable[SIDE_BLACK][pawnLoc] = BB::southWestOne(pawnAFileClearedBB) | BB::southEastOne(pawnHFileClearedBB);
    }

    // initializes move generation
    void init()
    {
        initRays();

        // initialize all of the lookup tables
        for (int pieceLoc = 0; pieceLoc < 64; pieceLoc++)
        {
            initKingLT(pieceLoc);
            initKnightLT(pieceLoc);
            initPawnLT(SIDE_BLACK, pieceLoc);
            initPawnLT(SIDE_WHITE, pieceLoc);
        }
    }

    // computes the king's pseudo moves based on the positions of friendly pieces
    Bitboard computePseudoKingMoves(Byte fromSquare, Bitboard friendlyPiecesBB)
    {
        // we already have predefined moves for the king so we need only index the correct element of the lookup table
        // then we simply ensure that we aren't allowing moves onto friendly pieces
        return kingLookupTable[fromSquare] & ~friendlyPiecesBB;
    }

    // computes the knight's pseudo moves based on the positions of friendly pieces
    Bitboard computePseudoKnightMoves(Byte fromSquare, Bitboard friendlyPiecesBB)
    {
        // same process as above
        return knightLookupTable[fromSquare] & ~friendlyPiecesBB;
    }

    // computes the pawn's pseudo moves based on the positions of other pieces
    Bitboard computePseudoPawnMoves(Byte fromSquare, Colour side, Bitboard enemyPiecesBB, Bitboard emptyBB, Bitboard enPassantBB)
    {
        // initialize the moves bitboard with the possible diagonal moves that the pawn can make
        // it's important to note that pawn's can only attack diagonally when an enemy occupies the square
        Bitboard movesBB = (pawnAttackLookupTable[side][fromSquare] & enemyPiecesBB) | (pawnAttackLookupTable[side][fromSquare] & enPassantBB);

        // here we're adding the square in front of the pawn to its possibles moves, should it not be blocked by any other pieces
        // additionally, we add the square two in front of the pawn if it isn't blocked and the pawn was on its home rank
        if (side == SIDE_WHITE)
        {
            Bitboard oneStepBB = BB::northOne(BB::boardSquares[fromSquare]) & emptyBB;

            // if the twostep is on the fourth rank, it would mean the pawn was on its home rank
            Bitboard twoStepBB = ((BB::northOne(oneStepBB)) & BB::rankMask[BB::RANK_FOURTH]) & emptyBB;
            
            movesBB |= oneStepBB | twoStepBB;
        }
        else if (side == SIDE_BLACK)
        {
            Bitboard oneStepBB = BB::southOne(BB::boardSquares[fromSquare]) & emptyBB;

            // if the twostep is on the fifth rank, it would mean the pawn was on its home rank
            Bitboard twoStepBB = ((BB::southOne(oneStepBB)) & BB::rankMask[BB::RANK_FIFTH]) & emptyBB;

            movesBB |= oneStepBB | twoStepBB;
        }

        return movesBB;
    }

    // this function sets a bit on the Bitboard according to the square passed into function if
    // the square was empty (meaning the sliding piece can move to it) or the square has an enemy
    // piece on it (meaning the sliding piece can capture it)
    // returns true when the sliding piece must halt due to a piece blocking it
    inline bool slidingPieceMoveStep(int square, Bitboard* movesBB, Bitboard enemyPiecesBB, Bitboard friendlyPiecesBB)
    {
        // checks if the square is occupied by a friendly piece, meaning we should immediately return
        if (BB::boardSquares[square] & friendlyPiecesBB)
            return true;

        // if the square was not occupied by a friendly piece, then we can for sure add it to the moves bitboard,
        // as even if it is occupied by an enemy piece, the sliding piece would be able to capture it (and therefore
        // be able to move to that square)
        *movesBB |= BB::boardSquares[square];

        if (BB::boardSquares[square] & enemyPiecesBB)
            return true;

        return false;
    }

    // Uses the classical approach to calculate rook (straight) moves
    // see more infomration on this implentation here: https://www.chessprogramming.org/Classical_Approach
    Bitboard computePseudoRookMoves(Byte fromSquare, Bitboard occupiedBB, Bitboard friendlyPiecesBB)
    {
        // return immediately if the square that the piece is on is invalid (off the board)
        if (fromSquare > 64)
            return 0;

        Bitboard movesBB = 0;

        // compute the Bitboard for the moves that the rook can make in its north direction
        Bitboard currentMoves = rays[DIR_NORTH][fromSquare];
        Bitboard blockers = currentMoves & occupiedBB;
        if (blockers)
            currentMoves ^= rays[DIR_NORTH][BB::getLSB(blockers)];
        movesBB |= currentMoves;

        // compute the Bitboard for the moves that the rook can make in its south direction
        currentMoves = rays[DIR_SOUTH][fromSquare];
        blockers = currentMoves & occupiedBB;
        if (blockers)
            currentMoves ^= rays[DIR_SOUTH][BB::getMSB(blockers)];
        movesBB |= currentMoves;

        // compute the Bitboard for the moves that the rook can make in its west direction
        currentMoves = rays[DIR_WEST][fromSquare];
        blockers = currentMoves & occupiedBB;
        if (blockers)
            currentMoves ^= rays[DIR_WEST][BB::getMSB(blockers)];
        movesBB |= currentMoves;

        // compute the Bitboard for the moves that the rook can make in its east direction
        currentMoves = rays[DIR_EAST][fromSquare];
        blockers = currentMoves & occupiedBB;
        if (blockers)
            currentMoves ^= rays[DIR_EAST][BB::getLSB(blockers)];
        movesBB |= currentMoves;

        // returning this expression ensures that the moves bitboard will not include squares occupied by friendly pieces
        return movesBB & ~friendlyPiecesBB;
    }

    
    // Uses the classical approach to calculate bishop (diagonal) moves
    // see more infomration on this implentation here: https://www.chessprogramming.org/Classical_Approach
    Bitboard computePseudoBishopMoves(Byte fromSquare, Bitboard occupiedBB, Bitboard friendlyPiecesBB)
    {
        // return immediately if the square that the piece is on is invalid (off the board)
        if (fromSquare > 64)
            return 0;

        Bitboard movesBB = 0;

        // compute the Bitboard for the moves that the rook can make in its northeast direction
        Bitboard currentMoves = rays[DIR_NORTHEAST][fromSquare];
        Bitboard blockers = currentMoves & occupiedBB;
        if (blockers)
            currentMoves ^= rays[DIR_NORTHEAST][BB::getLSB(blockers)];
        movesBB |= currentMoves;

        // compute the Bitboard for the moves that the rook can make in its nortwest direction
        currentMoves = rays[DIR_NORTHWEST][fromSquare];
        blockers = currentMoves & occupiedBB;
        if (blockers)
            currentMoves ^= rays[DIR_NORTHWEST][BB::getLSB(blockers)];
        movesBB |= currentMoves;

        // compute the Bitboard for the moves that the rook can make in its southeast direction
        currentMoves = rays[DIR_SOUTHEAST][fromSquare];
        blockers = currentMoves & occupiedBB;
        if (blockers)
            currentMoves ^= rays[DIR_SOUTHEAST][BB::getMSB(blockers)];
        movesBB |= currentMoves;

        // compute the Bitboard for the moves that the rook can make in its southwest direction
        currentMoves = rays[DIR_SOUTHWEST][fromSquare];
        blockers = currentMoves & occupiedBB;
        if (blockers)
            currentMoves ^= rays[DIR_SOUTHWEST][BB::getMSB(blockers)];
        movesBB |= currentMoves;

        // returning this expression ensures that the moves bitboard will not include squares occupied by friendly pieces
        return movesBB & ~friendlyPiecesBB;
    }

    // this function computes a queen's moves bitboard by simply ORING the moves that a rook and bishop could make from its square
    Bitboard computePseudoQueenMoves(Byte fromSquare, Bitboard occupiedBB, Bitboard friendlyPiecesBB)
    {
        return computePseudoBishopMoves(fromSquare, occupiedBB, friendlyPiecesBB) | computePseudoRookMoves(fromSquare, occupiedBB, friendlyPiecesBB);
    }

    // sets the castle privileges that a move will remove (for instance, a white piece taking a rook that hasn't moved
    // would remove the black side from castling on that rook's side)
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

    // calculates the castle moves that the given side can make according to its privileges
    MoveData computeCastleMoveData(Colour side, Byte privileges, Bitboard occupiedBB, CastlingPrivilege castleType)
    {
        // setting the initial values for the move
        MoveData md;
        md.side = side;

        // note that setting the move type to regular here is necessary, as we can use the move type of the move 
        // as a flag to see if the castle is legal
        md.moveType = MoveType::REGULAR;

        // these variables store the starting and ending square that the king must be able to move between to make a castle
        int lower, higher;

        switch (castleType)
        {
            // check if the move being attempted is a short (kingside) castle
            case CastlingPrivilege::WHITE_SHORT_CASTLE:
            case CastlingPrivilege::BLACK_SHORT_CASTLE:
            {

                // check if the side to move has the privileges it needs to short castle
                if (((privileges & (Byte)CastlingPrivilege::WHITE_SHORT_CASTLE) && side == SIDE_WHITE) ||
                    (privileges & (Byte)CastlingPrivilege::BLACK_SHORT_CASTLE) && side == SIDE_BLACK)
                {
                    // set the starting and ending squares that must be viable for the king
                    if (side == SIDE_WHITE) { lower = ChessCoord::F1; higher = ChessCoord::H1; }
                    if (side == SIDE_BLACK) { lower = ChessCoord::F8; higher = ChessCoord::H8; }

                    // check if the squares in the bounds defined above are occupied, and if they are, do not allow the castle
                    for (int tile = lower; tile < higher; tile++)
                        if (BB::boardSquares[tile] & occupiedBB)
                        {
                            md.moveType = MoveType::INVALID;
                            break;
                        }

                    // checks to see if the castle move is possible (it will be as long as the move's type hasn't been set to invalid)
                    if (md.moveType != MoveType::INVALID)
                    {
                        md.moveType = MoveType::SHORT_CASTLE;
                        setCastleMovePrivilegesRevoked(md.side, privileges, &md.castlePrivilegesRevoked);

                        // sets the origin and square for the move for the king
                        md.originSquare = lower - 1;
                        md.targetSquare = higher - 1;
                    }
                }

                break;
            }

            // check if the move being attempted is a long (queenside) castle
            case CastlingPrivilege::WHITE_LONG_CASTLE:
            case CastlingPrivilege::BLACK_LONG_CASTLE:
            {
                // check if the side to move has the privileges it needs to long castle
                if (((privileges & (Byte)CastlingPrivilege::WHITE_LONG_CASTLE) && side == SIDE_WHITE) ||
                    (privileges & (Byte)CastlingPrivilege::BLACK_LONG_CASTLE) && side == SIDE_BLACK)
                {
                    // set the starting and ending squares that must be viable for the king
                    if (side == SIDE_WHITE) { lower = ChessCoord::A1;  higher = ChessCoord::D1; }
                    if (side == SIDE_BLACK) { lower = ChessCoord::A8; higher = ChessCoord::D8; }

                    // check if the squares in the bounds defined above are occupied, and if they are, do not allow the castle
                    for (int tile = higher; tile > lower; tile--)
                        if (BB::boardSquares[tile] & occupiedBB)
                        {
                            md.moveType = MoveType::INVALID;
                            break;
                        }

                    // checks to see if the castle move is possible (it will be as long as the move's type hasn't been set to invalid)
                    if (md.moveType != MoveType::INVALID)
                    {
                        md.moveType = MoveType::LONG_CASTLE;
                        setCastleMovePrivilegesRevoked(md.side, privileges, &md.castlePrivilegesRevoked);

                        // sets the origin and square for the move for the king
                        md.originSquare = higher + 1;
                        md.targetSquare = lower + 2;
                    }
                }

                break;
            }
        }

        // if the move type is regular, which would indicate that the castle move was unsuccessful 
        // (likely due to no privileges), then we need to set the move to invalid so it isn't added
        // to the move vector
        if (md.moveType == MoveType::REGULAR)
            md.moveType = MoveType::INVALID;

        return md;
    }

    // this function generates all the moves like any other call to calculateSideMoves, but it only adds 
    // moves that involve captures to the move vector
    void calculateCaptureMoves(Board* board, Colour side, std::vector<MoveData>& moveVec)
    {
        calculateSideMoves(board, side, moveVec, true);
    }

    // returns the piece bitboard that the piece on the given square belongs to
    // the side parameter is a default arguement with a value of -1. this value of -1 indicates no side was specified and to therefore search all bitboards
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

    // uses pointer arguements to return the piece bitboard and the value of the piece, given a square on the board
    void getPieceData(Board* boardPtr, Bitboard** pieceBB, int* pieceValue, Byte square, Colour side)
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

    // generates all of the possible (pseudo) moves that the side to make can be made with the current position 
    // the MoveData vector that is passed in by reference is filled with all these possible moves
    void calculateSideMoves(Board* board, Colour side, std::vector<MoveData>& moveVec, bool captureOnly)
    {
        // clear the move vector and reserve some space for up to 64 moves (just so that there is no performance 
        // hit when the move vector has to constantly reallocate more and more space)
        moveVec.clear();
        moveVec.reserve(64);

        // contains the colour bitboard of the side to move
        Bitboard colourBB = side == SIDE_WHITE ? board->currentPosition.whitePiecesBB : board->currentPosition.blackPiecesBB;

        // calculautes all of the possible moves for the pieces on each square occupied by the side to move
        for (int square = 0; square < 64; square++)
            if (BB::boardSquares[square] & colourBB)
                calculatePieceMoves(board, side, square, moveVec, captureOnly);

        // calculate as well any castle moves (IF we are generating all moves, and not just castle moves)
        if (!captureOnly)
            calculateCastleMoves(board, side, moveVec);
    }

    // generates all of the possible castle moves based on the side that's moving, and fills said castle
    // moves into the move vector passed in by reference
    void calculateCastleMoves(Board* board, Colour side, std::vector<MoveData>& movesVec)
    {
        MoveData shortCastleMD, longCastleMD;

        if (side == SIDE_WHITE)
        {
            // if the king or rook bitboards are empty, then no castling moves will be possible for this side
            if (board->currentPosition.whiteKingBB == 0 || board->currentPosition.whiteRooksBB == 0)
                return;

            // generate the short and long castle moves (if they aren't possible, these will simply return moves with INVALID move types)
            shortCastleMD = computeCastleMoveData(side, board->currentPosition.castlePrivileges, board->currentPosition.occupiedBB, CastlingPrivilege::WHITE_SHORT_CASTLE);
            longCastleMD  = computeCastleMoveData(side, board->currentPosition.castlePrivileges, board->currentPosition.occupiedBB, CastlingPrivilege::WHITE_LONG_CASTLE);
        }
        else
        {
            // if the king or rook bitboards are empty, then no castling moves will be possible for this side
            if (board->currentPosition.blackKingBB == 0 || board->currentPosition.blackRooksBB == 0)
                return;

            // generate the short and long castle moves (if they aren't possible, these will simply return moves with INVALID move types)
            shortCastleMD = computeCastleMoveData(side, board->currentPosition.castlePrivileges, board->currentPosition.occupiedBB, CastlingPrivilege::BLACK_SHORT_CASTLE);
            longCastleMD  = computeCastleMoveData(side, board->currentPosition.castlePrivileges, board->currentPosition.occupiedBB, CastlingPrivilege::BLACK_LONG_CASTLE);
        }

        // these if statements will add the castle moves to the move vector if they were psuedo legal
        if (shortCastleMD.moveType != MoveType::INVALID)
            movesVec.push_back(shortCastleMD);
        if (longCastleMD.moveType != MoveType::INVALID)
            movesVec.push_back(longCastleMD);
    }

    /*
        changes the move provided in the arguement's data based on whether or not the move will capture a rook
        due to how bitwise operators are used to change the castling privileges when a move is made or unmade,
        some additional factors need to be considered when setting the castlePrivilegesRevoked attribute of the 
        move's data. for instance, we should only revoke short castling privileges if the rook was in its starting
        position and the other side had permission to castle on the short side at the time of this move. the same goes
        for the long castling privileges
        returns true if the provided move will cause castle privileges to be revoked
    */
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

    // 
    inline void setEnPassantMoveData(Board* board, int square, Bitboard pieceMovesBB, MoveData* md)
    {
        // cheks if an en passant square is currently set on the board (i.e., there is a square that a pawn could move on via the en passant rule)
        if (board->currentPosition.enPassantSquare < 64)
        {
            // checks to see if the square provided is the same as the en passant square currently set on the board
            if (BB::boardSquares[square] & BB::boardSquares[board->currentPosition.enPassantSquare])
            {
                // passes if the moves bitboard provided contains the en passant square in its possible moves
                if (pieceMovesBB & BB::boardSquares[board->currentPosition.enPassantSquare])
                {
                    /*  
                        the following two if statements will pass if the piece making the move is a pawn
                        if it is, it means that the move will be an en passant capture (as it is a pawn moving onto the en passant square)
                        in this case, we need to set the appropriate data in the move's data, chiefly:
                            - the pawn that will be captured by the en passant (notice below that it is calculated 
                              by finding the enemy piece that is 1 square in front of the en passant square)
                            - the move type must be set to en passant capture so the move can be properly made and unmade
                    */
                    if (md->pieceBB == &board->currentPosition.whitePawnsBB)
                    {
                        md->capturedPieceBB = getPieceBitboard(board, square - 8, SIDE_BLACK);
                        md->moveType = MoveType::EN_PASSANT_CAPTURE;
                    }
                    else if (md->pieceBB == &board->currentPosition.blackPawnsBB)
                    {
                        md->capturedPieceBB = getPieceBitboard(board, square + 8, SIDE_WHITE);
                        md->moveType = MoveType::EN_PASSANT_CAPTURE;
                    }
                }
            }

            // the move data always contains the en passant square that is set on the board
            // this is so that when the move is unmade, the en passant square that was set
            // on the board can be restored
            md->enPassantSquare = board->currentPosition.enPassantSquare;
        }
    }

    // uses the calculated moves bitboard to add the actual moves that have been abstracted into the engine (with all the 
    // data necessary to make and unmake moves) to the MoveData vector provided
    void addMoves(Board* board, Bitboard movesBB, MoveData& md, std::vector<MoveData>& moveVec, bool captureOnly)
    {
        for (int square = 0; square < 64; square++)
        {
            // checks to see if the square on the board is one of the squares that the moves bitboard contains
            // this would mean that we need to convert this into an actual move that the engine can use
            if (movesBB & BB::boardSquares[square])
            {
                // set the default values for the move data
                md.targetSquare = square;
                md.capturedPieceBB = nullptr;
                md.moveType = MoveType::REGULAR;

                // if there is an enemy piece on the square that the piece is moving to, get the data bout the piece that would be captured
                if (BB::boardSquares[square] & *md.capturedColourBB)
                    getPieceData(board, &md.capturedPieceBB, &md.capturedPieceValue, square, !md.side);

                // if there is no captured piece and we are only adding moves that are capture moves, then continue to the next move
                if (captureOnly && !md.capturedPieceBB)
                    continue;

                // these if statements check to see if a pawn would be promoted by this move
                if (md.pieceBB == &board->currentPosition.whitePawnsBB && md.targetSquare >= ChessCoord::A8) md.moveType = MoveType::PAWN_PROMOTION;
                if (md.pieceBB == &board->currentPosition.blackPawnsBB && md.targetSquare <= ChessCoord::H1) md.moveType = MoveType::PAWN_PROMOTION;

                setEnPassantMoveData(board, square, movesBB, &md);

                // we need to reset the privileges revoked after a move that captures a rook has changed them
                // otherwise, no matter what move the piece makes (i.e. no capturing the rook), the moves 
                // castlePrivilegesRevoked attribute will be non-zero (and change the castle privileges)
                // calling this functions also sets which castlePrivileges will be revoked after making the move
                bool resetCastlePrivileges = doesCaptureAffectCastle(board, &md);

                moveVec.push_back(md);

                if (resetCastlePrivileges)
                    md.castlePrivilegesRevoked = 0;
            }
        }
    }

    // this function will cause certain castle privileges to be revoked based on the move. for instance, the king moving from its starting square (for the first time)
    // or one of the rooks moving from its starting square (for the first time) will revoke all or some of that side's castle privileges
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

    // returns the moves bitboard that the piece on a given square can make
    // each if statement in the function essentially just checks which type of piece the piece is, and then
    // calls the appropriate function that calculates the moves bitboard for the given piece type
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

        // rooks and kings moving may changae the castle privileges of the side moving, so we need to set the proper 
        // castle privileges based on the move that's being made
        else if ((pieceBB & board->currentPosition.whiteRooksBB) || (pieceBB & board->currentPosition.blackRooksBB))
        {
            setCastlePrivileges(board, md, false);
            return computePseudoRookMoves(md->originSquare, board->currentPosition.occupiedBB, *md->colourBB);
        }
        else if ((pieceBB & board->currentPosition.whiteKingBB) || (pieceBB & board->currentPosition.blackKingBB))
        {
            setCastlePrivileges(board, md, true);
            return computePseudoKingMoves(md->originSquare, *md->colourBB);
        }
    }

    // add the moves that a single piece can make to the move vector
    void calculatePieceMoves(Board* board, Colour side, Byte originSquare, std::vector<MoveData>& moveVec, bool captureOnly)
    {
        // the following code sets the default values for the move's data 

        MoveData md;
        md.colourBB         = side == SIDE_WHITE ? &board->currentPosition.whitePiecesBB : &board->currentPosition.blackPiecesBB;
        md.capturedColourBB = side == SIDE_WHITE ? &board->currentPosition.blackPiecesBB : &board->currentPosition.whitePiecesBB;
        md.side = side;
        md.originSquare = originSquare;

        Bitboard movesBB = 0;
        getPieceData(board, &md.pieceBB, &md.pieceValue, originSquare, side);

        // get the moves bitboard for the piece
        movesBB = calculatePsuedoMove(board, &md, *md.pieceBB);

        // add the moves to the move vector by converting the moves from bitboards to the engine's abstraction of a move
        // (but only if there are any moves to )
        if (movesBB > 0)
            addMoves(board, movesBB, md, moveVec, captureOnly);
    }
}