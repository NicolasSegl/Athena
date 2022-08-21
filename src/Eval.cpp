#include "Bitboard.h"
#include "Board.h"
#include "Eval.h"
#include "SquarePieceTables.h"
#include "utils.h"

namespace Eval
{
    // returns the evaluation of the board's position relative to the specified side
    int evaluateBoardRelativeTo(Colour side, int eval)
    {
        return eval * (1 - 2 * (Byte)side);
    }

    float getMidgameValue(Bitboard occupiedBB)
    {     return countSetBits64(occupiedBB)/32.f; // 32 = number of total pieces possible
    }

    // calculates the value of a pawn based on its structure
    int evaluatePawnValue(int square, Bitboard pawnsBB)
    {
        
        int structureEval = 0;

        //else // if blocked by a piece that is not a pawn

        // blocked pawn penalty
        //if ()

        // double/triple pawn penalty. apply when pawns are on the same file (applied per pawn. so a doubled pawn is a penalty of -10*2=-20)
        if ((~BB::rankClear[square / 8] ^ BB::boardSquares[square]) & pawnsBB)
            structureEval -= 10;

        // isolated pawns
        bool isolatedLeftFile = true, isolatedRightFile = true;

        if ((square % 8) - 1 < 0) // if the pawn is not on the a file
            if (~BB::fileClear[square % 8 - 1] & pawnsBB) // if there is a pawn on the file to the left
                isolatedLeftFile = false;

        if (isolatedLeftFile) // if the file to the left has no pawn, then check the right file
            if ((square % 8) + 1 > 7) // if the pawn is not on the h file
                if (~BB::fileClear[square % 8 + 1] & pawnsBB) // if there is a pawn on the adjacent right file
                    isolatedRightFile = false;

        if (isolatedRightFile && isolatedLeftFile)
            structureEval -= 20;
        // if the pawn is not isolated, it may be backwards
        else
        {
            // if the pawn is BEHIND (higher/lower rank) than
            //if (side == SIDE_WHITE)
             //   if (BB::boardSquares[square] < )
        }

        // backward pawns
        

         return 0;
    }

    int evaluatePosition(Board* boardPtr, float midgameValue)
    {
        int whiteEval = 0;
        int blackEval = 0;
        
        for (int square = 0; square < 64; square++)
        {
            if (BB::boardSquares[square] & boardPtr->currentPosition.emptyBB) // optimization
                continue;

            // consider piece value and piece square table
            if (BB::boardSquares[square] & boardPtr->currentPosition.whitePiecesBB)
            {
                if (BB::boardSquares[square] & boardPtr->currentPosition.whitePawnsBB)
                {
                    whiteEval += PAWN_VALUE + pst::pawnTable[63 - square] + evaluatePawnValue(square, boardPtr->currentPosition.whitePawnsBB);
                }
                else if (BB::boardSquares[square] & boardPtr->currentPosition.whiteKnightsBB) whiteEval += KNIGHT_VALUE + pst::knightTable[63 - square];
                else if (BB::boardSquares[square] & boardPtr->currentPosition.whiteBishopsBB) whiteEval += BISHOP_VALUE + pst::bishopTable[63 - square];
                else if (BB::boardSquares[square] & boardPtr->currentPosition.whiteRooksBB)   whiteEval += ROOK_VALUE + pst::rookTable[63 - square];
                else if (BB::boardSquares[square] & boardPtr->currentPosition.whiteQueensBB)  whiteEval += QUEEN_VALUE + pst::queenTable[63 - square];
                else if (BB::boardSquares[square] & boardPtr->currentPosition.whiteKingBB)
                    // so a pawn hash table is just a transposition table but for pawn structures??
                    whiteEval += KING_VALUE + pst::midgameKingTable[63 - square] * midgameValue + pst::endgameKingTable[63 - square] * (1 - midgameValue);
            }
            else // piece is black
            {
                if (BB::boardSquares[square] & boardPtr->currentPosition.blackPawnsBB)   blackEval += PAWN_VALUE + pst::pawnTable[square];
                else if (BB::boardSquares[square] & boardPtr->currentPosition.blackKnightsBB) blackEval += KNIGHT_VALUE + pst::knightTable[square];
                else if (BB::boardSquares[square] & boardPtr->currentPosition.blackBishopsBB) blackEval += BISHOP_VALUE + pst::bishopTable[square];
                else if (BB::boardSquares[square] & boardPtr->currentPosition.blackRooksBB)   blackEval += ROOK_VALUE + pst::rookTable[square];
                else if (BB::boardSquares[square] & boardPtr->currentPosition.blackQueensBB)  blackEval += QUEEN_VALUE + pst::queenTable[square];
                else if (BB::boardSquares[square] & boardPtr->currentPosition.blackKingBB)
                    blackEval += KING_VALUE + pst::midgameKingTable[square] * midgameValue + pst::endgameKingTable[square] * (1 - midgameValue);
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
