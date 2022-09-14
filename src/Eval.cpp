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
    const int DOUBLE_PAWN_PENALTY   = 10;
    const int ISOLATED_PAWN_PENALTY = 20;
    const int PASSED_PAWN_BONUS     = 30;

    // rook structure values
    const int ROOK_OPEN_FILE_BONUS      = 30;
    const int ROOK_HALF_OPEN_FILE_BONUS = 20;

    // king midgame structure values
    const int KING_MIDGAME_OPEN_FILE_PENALTY = 50;

    // returns the evaluation of the board's position relative to the specified side
    int evaluateBoardRelativeTo(Colour side, int eval)
    {
        return eval * (1 - 2 * (Byte)side);
    }

    float getMidgameValue(Bitboard occupiedBB)
    {
        return countSetBits64(occupiedBB)/32.f; // 32 = number of total pieces possible
    }

    struct PawnHashTableEntry
    {
        int structureEval;
        Bitboard pawnsBB = 0;
    };

    PawnHashTableEntry* pawnHashTable;
    const int PAWN_HASH_TABLE_SIZE = 1000000;
    void initPawnHashTable()
    {
        pawnHashTable = new PawnHashTableEntry[PAWN_HASH_TABLE_SIZE];
    }

    // calculates the value of a pawn based on its structure
    int evaluatePawnStructure(Bitboard friendlyPawnsBB, Bitboard enemyPawnsBB)
    {
        if (!friendlyPawnsBB)
            return 0;
        
        // if there is an entry with the same
        int hashEntryIndex = friendlyPawnsBB % PAWN_HASH_TABLE_SIZE;
        if (pawnHashTable[hashEntryIndex].pawnsBB == friendlyPawnsBB)
            return pawnHashTable[hashEntryIndex].structureEval;

        int structureEval = 0;
        for (int square = 0; square < 64; square++)
        {
            if (BB::boardSquares[square] & friendlyPawnsBB)
            {
                // double/triple pawn penalty. apply when pawns are on the same file (applied per pawn. so a doubled pawn is a penalty of -10*2=-20)
                if ((BB::fileMask[square % 8] & ~BB::boardSquares[square]) & friendlyPawnsBB)
                    structureEval -= DOUBLE_PAWN_PENALTY;

                // isolated pawns
                if (!(BB::adjacentFiles[square % 8] & friendlyPawnsBB))
                    structureEval -= ISOLATED_PAWN_PENALTY;
                // if the pawn is not isolated, it may be backwards
                else
                {
                    // if the adjacent pawns are a rank ahead of the considered pawn
                    // AND the pawn cannot move up, 
                }
                
                // test speed up when using file/rank masks instead of ~file or rank clears
                // passed pawns
                if (!((BB::adjacentFiles[square % 8] | BB::fileMask[square % 8]) & enemyPawnsBB))
                    structureEval += PASSED_PAWN_BONUS;
            }
        }
        
        pawnHashTable[hashEntryIndex].pawnsBB          = friendlyPawnsBB;
        pawnHashTable[hashEntryIndex].structureEval    = structureEval;

        return structureEval;
    }

    int kingMidgameStructureValue(int square, Bitboard friendlyPiecesBB)
    {
        // note that the piece square tables already accomodate for pawn shields in the midgame
        int value = 0;

        // open file next to king
        if (square % 8 > 0)
        {
            if (!(BB::westFile[square % 8] & friendlyPiecesBB))
                value -= KING_MIDGAME_OPEN_FILE_PENALTY;
        }
        if (square % 8 < 7)
        {
            if (!(BB::eastFile[square % 8] & friendlyPiecesBB))
                value -= KING_MIDGAME_OPEN_FILE_PENALTY;
        }

       return value;
    }

    int kingStructureValue(int square, int pstIndex, Bitboard friendlyPiecesBB, float midgameValue)
    {
        int midgameValueScaled = pst::midgameKingTable[pstIndex] * midgameValue + kingMidgameStructureValue(square, friendlyPiecesBB) * midgameValue;
        int endgameValueScaled = pst::endgameKingTable[pstIndex] * (1 - midgameValue);
        return midgameValueScaled + endgameValueScaled;

    }

    inline int rookStructureValue(int square, Bitboard occupiedBB, Bitboard friendlyPiecesBB, Bitboard enemyPiecesBB, Bitboard friendlyRooksBB)
    {
        Bitboard bitsSetInFile = BB::fileMask[square % 8] & ~BB::boardSquares[square];
        int value = 0;

        // rooks are connected
        if (MoveGeneration::computePseudoRookMoves(square, friendlyPiecesBB | enemyPiecesBB, friendlyPiecesBB) & friendlyRooksBB)
            value += 10;
        
        // open file
        if (!(bitsSetInFile & occupiedBB))
            value += ROOK_OPEN_FILE_BONUS;
        else // potentially a half-open file
            if (!(bitsSetInFile & friendlyPiecesBB) && (bitsSetInFile & enemyPiecesBB))
                value += ROOK_HALF_OPEN_FILE_BONUS;

        return value;
    }

    int evaluatePosition(Board* boardPtr, float midgameValue)
    {
        ChessPosition& position = boardPtr->currentPosition;
        
        int whiteEval = evaluatePawnStructure(position.whitePawnsBB, position.blackPawnsBB);
        int blackEval = evaluatePawnStructure(position.blackPawnsBB, position.whitePawnsBB);
        
        for (int square = 0; square < 64; square++)
        {
            if (BB::boardSquares[square] & position.emptyBB) // optimization
                continue;

            // consider piece value and piece square table
            if (BB::boardSquares[square] & boardPtr->currentPosition.whitePiecesBB)
            {
                if (BB::boardSquares[square] & position.whitePawnsBB)
                    whiteEval += PAWN_VALUE + pst::pawnTable[63 - square];
                else if (BB::boardSquares[square] & position.whiteKnightsBB) whiteEval += KNIGHT_VALUE + pst::knightTable[63 - square];
                else if (BB::boardSquares[square] & position.whiteBishopsBB) whiteEval += BISHOP_VALUE + pst::bishopTable[63 - square];
                else if (BB::boardSquares[square] & position.whiteRooksBB)   whiteEval += ROOK_VALUE + pst::rookTable[63 - square] + rookStructureValue(square, position.occupiedBB, position.whitePiecesBB, position.blackPiecesBB, position.whiteRooksBB);
                else if (BB::boardSquares[square] & position.whiteQueensBB)  whiteEval += QUEEN_VALUE + pst::queenTable[63 - square];
                else if (BB::boardSquares[square] & position.whiteKingBB)
                    // so a pawn hash table is just a transposition table but for pawn structures??
                    whiteEval += KING_VALUE + kingStructureValue(square, 63 - square, boardPtr->currentPosition.whitePiecesBB, midgameValue);
            }
            else // piece is black
            {
                if (BB::boardSquares[square] & position.blackPawnsBB) blackEval += PAWN_VALUE + pst::pawnTable[square];
                else if (BB::boardSquares[square] & position.blackKnightsBB) blackEval += KNIGHT_VALUE + pst::knightTable[square];
                else if (BB::boardSquares[square] & position.blackBishopsBB) blackEval += BISHOP_VALUE + pst::bishopTable[square];
                else if (BB::boardSquares[square] & position.blackRooksBB)   blackEval += ROOK_VALUE + pst::rookTable[square] + rookStructureValue(square, position.occupiedBB, position.blackPiecesBB, position.whitePiecesBB, position.blackRooksBB);
                else if (BB::boardSquares[square] & position.blackQueensBB)  blackEval += QUEEN_VALUE + pst::queenTable[square];
                else if (BB::boardSquares[square] & position.blackKingBB)
                    blackEval += KING_VALUE + kingStructureValue(square, square, boardPtr->currentPosition.blackPiecesBB, midgameValue);
            }
        }
        
        return whiteEval - blackEval;
    }

    // see (static search evaluation) determines if an exchange of pieces on a certain square is winning or losing
    // note that this function does not currently consider if a move would result in a checks
    int see(Board* boardPtr, Byte square, Colour attackingSide, int currentSquareValue)
    {
        int pieceValue;
        Bitboard* pieceBB = nullptr;
        Bitboard  attacksToSquareBB;
        boardPtr->getLeastValuableAttacker(square, attackingSide, &pieceValue, &pieceBB, &attacksToSquareBB);
        
        // right now our pieceBB does not actually point to the board's bb for some reason?

        if (!pieceBB) // if there are no more attackers
            return -currentSquareValue;
        else
        {
            if (pieceValue < currentSquareValue)
                return currentSquareValue - pieceValue; // exchange is winning
            else
            {
                int lsb = BB::getLSB(attacksToSquareBB);
                *pieceBB &= ~BB::boardSquares[lsb]; // "make" the move
                int score = -Eval::see(boardPtr, square, !attackingSide, pieceValue);
                *pieceBB |= BB::boardSquares[lsb];// "unmake" the move
                return score;
            }
        }

    }

}
