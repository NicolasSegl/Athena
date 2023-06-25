#include <iostream>

#include "Bitboard.h"
#include "Board.h"
#include "Constants.h"
#include "Eval.h"
#include "MoveGeneration.h"
#include "SquarePieceTables.h"
#include "utils.h"

namespace Eval
{
    // pawn structure values
    const int BLOCKED_PAWN_PENALTY   = 5;
    const int PAWN_DOUBLED_PENALTY   = 10;
    const int PAWN_ISOLATED_PENALTY  = 20;
    const int PAWN_PASSED_BONUS      = 30;

    // rook structure values
    const int CONNECTED_ROOK_BONUS      = 10;
    const int ROOK_OPEN_FILE_BONUS      = 30;
    const int ROOK_HALF_OPEN_FILE_BONUS = 20;

    // king midgame structure values
    const int KING_MIDGAME_OPEN_FILE_PENALTY = 50;
    const int KING_MIDGAME_PAWN_SHIELD_BONUS = 1;

    // pawn hash table 
    struct PawnHashTableEntry
    {
        int structureEval;
        Bitboard pawnsBB = 0;
    };
    PawnHashTableEntry* pawnHashTable;
    const int PAWN_HASH_TABLE_SIZE = 1000000;

    // contains the distances between any 2 squares (with no diagonal movement)
    int distFromTable[64][64];

    // returns the evaluation of the board's position relative to the specified side
    int evaluateBoardRelativeTo(Colour side, int eval)
    {
        return eval * (1 - 2 * (Byte)side);
    }

    // represents as a decimal how far into the midgame we are. A value of 1.0 indicates the start, and a value of 0.0 would represent endgame
    float getMidgameValue(Bitboard occupiedBB)
    {
        // 32 = number of total pieces possible
        return countSetBits64(occupiedBB)/32.f;
    }

    // dynamically allocates memory for the pawn hash table, equal in size to sizeof(PawnHashTableEntry) * PAWN_HASH_TABLE_SIZE
    void initPawnHashTable()
    {
        pawnHashTable = new PawnHashTableEntry[PAWN_HASH_TABLE_SIZE];
    }

    // initializes the values describing the distance between any 2 squares in the distFromTable
    void initDistFromTable()
    {
        for (int from = 0; from < 64; from++)
            for (int to = 0; to < 64; to++)
            {
                // find how many rows apart the two squares are
                int rowDist = abs((from % 8) - (to % 8));

                // find how many columns apart the two squares are
                int columnDist = abs((from / 8) - (to / 8));

                distFromTable[from][to] = rowDist + columnDist;
            }
    }

    // initializes the tables that are necessary for board evaluation
    void init()
    {
        initPawnHashTable();
        initDistFromTable();
    }

    // calculates the value of a pawn based on its structure
    int evaluatePawnStructure(Bitboard friendlyPawnsBB, Bitboard enemyPawnsBB, ChessPosition& position)
    {
        // if there are no pawns left, return an evaluation of 0
        if (!friendlyPawnsBB)
            return 0;
        
        // if there is an entry with the same hash, we can use that entry's value for the pawn's evaluation
        int hashEntryIndex = friendlyPawnsBB % PAWN_HASH_TABLE_SIZE;
        if (pawnHashTable[hashEntryIndex].pawnsBB == friendlyPawnsBB)
            return pawnHashTable[hashEntryIndex].structureEval;

        int structureValue = 0;
        for (int square = 0; square < 64; square++)
        {
            if (BB::boardSquares[square] & friendlyPawnsBB)
            {
                // double/triple pawn penalty. apply when pawns are on the same file (applied per pawn. so a doubled pawn is a penalty of -10 * 2 = -20)
                if ((BB::fileMask[square % 8] & ~BB::boardSquares[square]) & friendlyPawnsBB)
                    structureValue -= PAWN_DOUBLED_PENALTY;

                if ((friendlyPawnsBB == position.whitePawnsBB && (BB::northOne(BB::boardSquares[square]) & position.blackPiecesBB)) || 
                    (friendlyPawnsBB == position.blackPawnsBB && (BB::southOne(BB::boardSquares[square]) & position.whitePiecesBB)))
                    structureValue -= BLOCKED_PAWN_PENALTY;

                // isolated pawns penalty
                else if (!(BB::adjacentFiles[square % 8] & friendlyPawnsBB))
                    structureValue -= PAWN_ISOLATED_PENALTY;
                // if the pawn is not isolated, it may be backwards
                else
                {
                    // if the adjacent pawns are a rank ahead of the considered pawn
                    // AND the pawn cannot move up, 
                }
                
                // passed pawns bonus
                if (!((BB::adjacentFiles[square % 8] | BB::fileMask[square % 8]) & enemyPawnsBB))
                    structureValue += PAWN_PASSED_BONUS;
            }
        }
        
        // set the values we just calculated into the pawn hash table for faster future pawn evaluation
        pawnHashTable[hashEntryIndex].pawnsBB       = friendlyPawnsBB;
        pawnHashTable[hashEntryIndex].structureEval = structureValue;

        return structureValue;
    }

    // evaluates how strong the pawn shield around the king is for the white side by checking for pawns being in select locations
    int whiteKingShieldValue(int kingSquare, Bitboard friendlyPawnsBB)
    {
        int shieldValue = 0;

        // if the king has long castled (or is otherwise on the west side of the board)
        if (kingSquare <= ChessCoord::C1)
        {
            if (friendlyPawnsBB & BB::boardSquares[ChessCoord::A2]) shieldValue += KING_MIDGAME_PAWN_SHIELD_BONUS;
            if (friendlyPawnsBB & BB::boardSquares[ChessCoord::A3]) shieldValue += KING_MIDGAME_PAWN_SHIELD_BONUS;
            if (friendlyPawnsBB & BB::boardSquares[ChessCoord::B2]) shieldValue += KING_MIDGAME_PAWN_SHIELD_BONUS;
            if (friendlyPawnsBB & BB::boardSquares[ChessCoord::B3]) shieldValue += KING_MIDGAME_PAWN_SHIELD_BONUS;
            if (friendlyPawnsBB & BB::boardSquares[ChessCoord::C2]) shieldValue += KING_MIDGAME_PAWN_SHIELD_BONUS;
        }

        // if the king has short castled (or is otherwise on the east side of the board)
        else if (kingSquare >= ChessCoord::G1)
        {
            if (friendlyPawnsBB & BB::boardSquares[ChessCoord::H2]) shieldValue += KING_MIDGAME_PAWN_SHIELD_BONUS;
            if (friendlyPawnsBB & BB::boardSquares[ChessCoord::H3]) shieldValue += KING_MIDGAME_PAWN_SHIELD_BONUS;
            if (friendlyPawnsBB & BB::boardSquares[ChessCoord::G2]) shieldValue += KING_MIDGAME_PAWN_SHIELD_BONUS;
            if (friendlyPawnsBB & BB::boardSquares[ChessCoord::G3]) shieldValue += KING_MIDGAME_PAWN_SHIELD_BONUS;
            if (friendlyPawnsBB & BB::boardSquares[ChessCoord::F2]) shieldValue += KING_MIDGAME_PAWN_SHIELD_BONUS;
        }

        return shieldValue;
    }

    // evaluates how strong the pawn shield around the king is for the black side by checking for pawns being in select locations
    int blackKingShieldValue(int kingSquare, Bitboard friendlyPawnsBB)
    {
        int shieldValue = 0;

        // if the king has long castled (or is otherwise on the west side of the board)
        if (kingSquare <= ChessCoord::C8)
        {
            if (friendlyPawnsBB & BB::boardSquares[ChessCoord::A7]) shieldValue += KING_MIDGAME_PAWN_SHIELD_BONUS;
            if (friendlyPawnsBB & BB::boardSquares[ChessCoord::A6]) shieldValue += KING_MIDGAME_PAWN_SHIELD_BONUS;
            if (friendlyPawnsBB & BB::boardSquares[ChessCoord::B7]) shieldValue += KING_MIDGAME_PAWN_SHIELD_BONUS;
            if (friendlyPawnsBB & BB::boardSquares[ChessCoord::B6]) shieldValue += KING_MIDGAME_PAWN_SHIELD_BONUS;
            if (friendlyPawnsBB & BB::boardSquares[ChessCoord::C7]) shieldValue += KING_MIDGAME_PAWN_SHIELD_BONUS;
        }

        // if the king has short castled (or is otherwise on the east side of the board)
        else if (kingSquare >= ChessCoord::G8)
        {
            if (friendlyPawnsBB & BB::boardSquares[ChessCoord::H7]) shieldValue += KING_MIDGAME_PAWN_SHIELD_BONUS;
            if (friendlyPawnsBB & BB::boardSquares[ChessCoord::H6]) shieldValue += KING_MIDGAME_PAWN_SHIELD_BONUS;
            if (friendlyPawnsBB & BB::boardSquares[ChessCoord::G7]) shieldValue += KING_MIDGAME_PAWN_SHIELD_BONUS;
            if (friendlyPawnsBB & BB::boardSquares[ChessCoord::G6]) shieldValue += KING_MIDGAME_PAWN_SHIELD_BONUS;
            if (friendlyPawnsBB & BB::boardSquares[ChessCoord::F7]) shieldValue += KING_MIDGAME_PAWN_SHIELD_BONUS;
        }

        return shieldValue;
    }

    // evaluates the structural position of the king during the midgame
    // it should be noted that the piece square tables already accomodate for pawn shields in the midgame
    int kingMidgameStructureValue(int square, Colour side, Bitboard friendlyPiecesBB, Bitboard friendlyPawnsBB)
    {
        int structureValue = 0;

        /* penalties for if there is an open file next to king */

        // if the king is not on the A file
        if (square % 8 > 0)
        {
            if (!(BB::westFile[square % 8] & friendlyPiecesBB))
                structureValue -= KING_MIDGAME_OPEN_FILE_PENALTY;
        }
        // if the king is not on the H file
        if (square % 8 < 7)
        {
            if (!(BB::eastFile[square % 8] & friendlyPiecesBB))
                structureValue -= KING_MIDGAME_OPEN_FILE_PENALTY;
        }

        // consider as well the strength of the pawn shield around the king
        structureValue += side == SIDE_WHITE ? whiteKingShieldValue(square, friendlyPawnsBB) : blackKingShieldValue(square, friendlyPawnsBB);

        return structureValue;
    }

    // evaluate the structual position of the king, using different weightings for various bonuses/penalties based on how far in the game is
    int kingStructureValue(int square, int pstIndex, Colour side, Bitboard friendlyPiecesBB, Bitboard friendlyPawnsBB, float midgameValue)
    {
        // compute the value of the king's position if it were the midgame, then scale that weighting by how far into the game it is
        int midgameValueScaled = pst::midgameKingTable[pstIndex] * midgameValue + 
                                 kingMidgameStructureValue(square, side, friendlyPiecesBB, friendlyPawnsBB) * midgameValue;

        // compute the value of the king's position if it were the endgame, then scale that weighting by how far into the game it is
        int endgameValueScaled = pst::endgameKingTable[pstIndex] * (1 - midgameValue);

        return midgameValueScaled + endgameValueScaled;
    }

    // evaluates the structural position of a rook
    inline int rookStructureValue(int square, Bitboard occupiedBB, Bitboard friendlyPiecesBB, Bitboard enemyPiecesBB, Bitboard friendlyRooksBB)
    {
        // find which squres are occupied on the rook's file (excluding the square on which the rook is)
        Bitboard bitsSetInFile = BB::fileMask[square % 8] & ~BB::boardSquares[square];

        int structureValue = 0;

        // connected rooks bonus
        if (MoveGeneration::computePseudoRookMoves(square, friendlyPiecesBB | enemyPiecesBB, friendlyPiecesBB) & friendlyRooksBB)
            structureValue += CONNECTED_ROOK_BONUS;
        
        // open file bonus
        if (!(bitsSetInFile & occupiedBB))
            structureValue += ROOK_OPEN_FILE_BONUS;
        else // if there is no open file, there might instead be a half-open file
            if (!(bitsSetInFile & friendlyPiecesBB) && (bitsSetInFile & enemyPiecesBB))
                structureValue += ROOK_HALF_OPEN_FILE_BONUS; // apply the half open file bonus

        return structureValue;
    }

    // evaluates the position of the entire board
    int evaluatePosition(Board* boardPtr, float midgameValue)
    {
        ChessPosition& position = boardPtr->currentPosition;
        
        // initialize the white and black side's evaluation using the evaluation for their pawn structures
        int whiteEval = evaluatePawnStructure(position.whitePawnsBB, position.blackPawnsBB, position);
        int blackEval = evaluatePawnStructure(position.blackPawnsBB, position.whitePawnsBB, position);
        
        for (int square = 0; square < 64; square++)
        {
            // if the square is empty, we need not consider it (it has no evaluation)
            if (BB::boardSquares[square] & position.emptyBB)
                continue;

            // the below if statements add to their respective side's total evaluation the worth of an individual piece based on their
            // material value as well as their position and structure
            if (BB::boardSquares[square] & boardPtr->currentPosition.whitePiecesBB)
            {
                if (BB::boardSquares[square] & position.whitePawnsBB)        whiteEval += PAWN_VALUE + pst::pawnTable[63 - square];
                else if (BB::boardSquares[square] & position.whiteKnightsBB) whiteEval += KNIGHT_VALUE + pst::knightTable[63 - square];
                else if (BB::boardSquares[square] & position.whiteBishopsBB) whiteEval += BISHOP_VALUE + pst::bishopTable[63 - square];
                else if (BB::boardSquares[square] & position.whiteRooksBB)   whiteEval += ROOK_VALUE + pst::rookTable[63 - square] + rookStructureValue(square, position.occupiedBB, position.whitePiecesBB, position.blackPiecesBB, position.whiteRooksBB);
                else if (BB::boardSquares[square] & position.whiteQueensBB)  whiteEval += QUEEN_VALUE + pst::queenTable[63 - square];
                else if (BB::boardSquares[square] & position.whiteKingBB)
                    // so a pawn hash table is just a transposition table but for pawn structures??
                    whiteEval += KING_VALUE + kingStructureValue(square, 63 - square, SIDE_WHITE, position.whitePiecesBB, position.whitePawnsBB, midgameValue);
            }
            else
            {
                if (BB::boardSquares[square] & position.blackPawnsBB)        blackEval += PAWN_VALUE + pst::pawnTable[square];
                else if (BB::boardSquares[square] & position.blackKnightsBB) blackEval += KNIGHT_VALUE + pst::knightTable[square];
                else if (BB::boardSquares[square] & position.blackBishopsBB) blackEval += BISHOP_VALUE + pst::bishopTable[square];
                else if (BB::boardSquares[square] & position.blackRooksBB)   blackEval += ROOK_VALUE + pst::rookTable[square] + rookStructureValue(square, position.occupiedBB, position.blackPiecesBB, position.whitePiecesBB, position.blackRooksBB);
                else if (BB::boardSquares[square] & position.blackQueensBB)  blackEval += QUEEN_VALUE + pst::queenTable[square];
                else if (BB::boardSquares[square] & position.blackKingBB)
                    blackEval += KING_VALUE + kingStructureValue(square, square, SIDE_BLACK, position.blackPiecesBB, position.blackPawnsBB, midgameValue);
            }
        }
        
        return whiteEval - blackEval;
    }

    // see (static search evaluation) determines if an exchange of pieces on a certain square is winning or losing
    // note that this function does not consider if a move would result in a check (making it not wholly accurate)
    int see(Board* boardPtr, Byte square, Colour attackingSide, int currentSquareValue)
    {
        // find the information about the least valuable attacker currently attacking the square
        int pieceValue;
        Bitboard* pieceBB = nullptr;
        Bitboard  attacksToSquareBB;
        boardPtr->getLeastValuableAttacker(square, attackingSide, &pieceValue, &pieceBB, &attacksToSquareBB);
        
        // if there is no piece bitboard, then there was no attacker found, and the square is no longer attacked
        if (!pieceBB)
            return -currentSquareValue;
        else
        {
            // checks if the exchange is winning (if material would be gained)
            if (pieceValue < currentSquareValue || !attacksToSquareBB)
                return currentSquareValue - pieceValue;
            
            // otherwise, if the exchange is losing, see if we would eventually win it down the line
            else
            {
                // removes the attacker from the potential list of attackers
                int lsb = BB::getLSB(attacksToSquareBB);
                *pieceBB &= ~BB::boardSquares[lsb];

                // search until we know for sure whether or not we'd eventually win the exchange
                int score = -Eval::see(boardPtr, square, !attackingSide, pieceValue);

                // adds the attacker back to the potential list of attackers
                *pieceBB |= BB::boardSquares[lsb];

                return score;
            }
        }
    }
}
