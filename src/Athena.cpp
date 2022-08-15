#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>

#include "Athena.h"
#include "Outcomes.h"
#include "SquarePieceTables.h"
#include "utils.h"

namespace Evaluation
{
    // values of the pieces relative to centipawns (1 pawn is worth 100 centipawns)
    const int KING_VALUE = 20000;
    const int QUEEN_VALUE = 900;
    const int ROOK_VALUE = 500;
    const int BISHOP_VALUE = 330;
    const int KNIGHT_VALUE = 320;
    const int PAWN_VALUE = 100;

    const int CHECKMATE_VALUE = 1000000;
}

const int INF = std::numeric_limits<int>::max();

// move ordering constants
const int MVV_LVA_OFFSET = 100;
const int KILLER_MOVE_SCORE = 10;
const int MAX_KILLER_MOVES = 2;

// initializes Athena's tranpsosition table and sets the default depth
Athena::Athena()
{   
    mDepth = 6;
    mMaxPly = 15;
    mTranspositionTable = new TranspositionHashEntry[mTranspositionTableSize];
    clearTranspositionTable();

    mKillerMoves = new MoveData*[mMaxPly];
    for (int i = 0; i < mMaxPly; i++)
    {
        mKillerMoves[i] = new MoveData[MAX_KILLER_MOVES];
        for (int j = 0; j < 2; j++)
            mKillerMoves[i][j] = {};
    }
}

// sets all the values in the transposition table to null (so we know that no data has yet been found at a given index)
void Athena::clearTranspositionTable()
{
    for (int i = 0; i < mTranspositionTableSize; i++)
        mTranspositionTable[i].hashFlag = TranspositionHashEntry::NONEXISTENT;
}

// calls negamax and returns the best move that it has found. also outputs some rudimentary data about the search
MoveData Athena::search(Board* ptr, float timeToMove)
{
    boardPtr = ptr;
    mNodes = 0;
    mMoveToMake.setMoveType(MoveData::EncodingBits::INVALID);

    auto beforeTime = std::chrono::steady_clock::now();
    std::cout << "max eval: " << negamax(mDepth, mSide, -INF, INF, 0) << std::endl;
    auto afterTime = std::chrono::steady_clock::now();
    std::cout << "time elapsed: " << std::chrono::duration<double>(afterTime - beforeTime).count() << std::endl;

    return mMoveToMake;
}

// finds an opening move from book.txt (file provided by tscp, an educational open source chess engine)
std::string Athena::getOpeningBookMove(Board* boardPtr, const std::vector<std::string>& lanStringHistory)
{
    std::ifstream bookFile;
    bookFile.open("engines/book.txt");

    // search through the whole file and pick the move that has the longest opening sequence
    std::string openingLine;
    std::string moveToMake = "";
    int longestLine = 0;
    while (std::getline(bookFile, openingLine))
    {
        // set mMoveToMake's origin and target squares
        std::vector<std::string> lineMoves;
        splitString(openingLine, lineMoves, ' ');
        
        if (lineMoves.size() > longestLine && lineMoves.size() > lanStringHistory.size())
        {
            // if the moves made so far in the game match up with this opening line's moves
            bool isActiveLine = true;
            for (int move = 0; move < lanStringHistory.size(); move++)
                if (lanStringHistory[move] != lineMoves[move])
                {
                    isActiveLine = false;
                    break;
                }

            // then set the next as the move to make
            if (isActiveLine)
            {
                moveToMake  = lineMoves[lanStringHistory.size()]; // indexes the current opening line move plus 1
                longestLine = lineMoves.size();
            }
        }
    }

    return moveToMake;
}

// calculates the value of a pawn based on its structure
int Athena::evaluatePawnValue(int square, Bitboard pawnBB)
{
    int structureEval = 0;

    // double pawn penalty
    // note that this works for both black and white pawns, as the penalty is calulcated just once (i.e. we do not apply the penalty
    // twice, one for each pawn doubled). likewise, if it's a triple pawn, we just actually do apply the penatly twice
    if (BB::boardSquares[square + 8] & pawnBB)
        structureEval -= 30; // relatively arbitrary number for now
    //else // if blocked by a piece that is not a pawn

    // blocked pawn penalty
    //if ()

    // double pawn penalty

    return 0;
}

int Athena::evaluatePosition(float midgameValue)
{
    int whiteEval = 0;
    int blackEval = 0;
    
    for (int square = 0; square < 64; square++)
    {
        if (BB::boardSquares[square] & boardPtr->currentPosition.emptyBB) // optimization
            continue;

        // for white pawns
        {
            // this is checking blocked pawns
            // if (northOne (i.e. << 8) & whitePawns), then SUBTRACT a value as a penalty

            // blocked pawns. pawns which have a piece direclty in front of them and cannot advance. apply a penalty
            // isolated pawns. pawns which do not have a friendly pawn defending it. apply a penalty
            // passed pawn. 
            // backward pawn
            // doubled pawn

        }

        // consider piece value and piece square table
        if (BB::boardSquares[square] & boardPtr->currentPosition.whitePawnsBB)
        {
            whiteEval += Evaluation::PAWN_VALUE + pst::pawnTable[63 - square];
        }
        else if (BB::boardSquares[square] & boardPtr->currentPosition.whiteKnightsBB) whiteEval += Evaluation::KNIGHT_VALUE + pst::knightTable[63 - square];
        else if (BB::boardSquares[square] & boardPtr->currentPosition.whiteBishopsBB) whiteEval += Evaluation::BISHOP_VALUE + pst::bishopTable[63 - square];
        else if (BB::boardSquares[square] & boardPtr->currentPosition.whiteRooksBB)   whiteEval += Evaluation::ROOK_VALUE + pst::rookTable[63 - square];
        else if (BB::boardSquares[square] & boardPtr->currentPosition.whiteQueensBB)  whiteEval += Evaluation::QUEEN_VALUE + pst::queenTable[63 - square];
        else if (BB::boardSquares[square] & boardPtr->currentPosition.whiteKingBB)
            // so a pawn hash table is just a transposition table but for pawn structures??
            whiteEval += Evaluation::KING_VALUE + pst::midgameKingTable[63 - square] * midgameValue + pst::endgameKingTable[63 - square] * (1 - midgameValue);

        else if (BB::boardSquares[square] & boardPtr->currentPosition.blackPawnsBB)   blackEval += Evaluation::PAWN_VALUE + pst::pawnTable[square];
        else if (BB::boardSquares[square] & boardPtr->currentPosition.blackKnightsBB) blackEval += Evaluation::KNIGHT_VALUE + pst::knightTable[square];
        else if (BB::boardSquares[square] & boardPtr->currentPosition.blackBishopsBB) blackEval += Evaluation::BISHOP_VALUE + pst::bishopTable[square];
        else if (BB::boardSquares[square] & boardPtr->currentPosition.blackRooksBB)   blackEval += Evaluation::ROOK_VALUE + pst::rookTable[square];
        else if (BB::boardSquares[square] & boardPtr->currentPosition.blackQueensBB)  blackEval += Evaluation::QUEEN_VALUE + pst::queenTable[square];
        else if (BB::boardSquares[square] & boardPtr->currentPosition.blackKingBB)
            blackEval += Evaluation::KING_VALUE + pst::midgameKingTable[square] * midgameValue + pst::endgameKingTable[square] * (1 - midgameValue);
    }
    
    return whiteEval - blackEval;
}

// this function is used to sort the moves. we need it to be called for every iteration of the move loop
// because we only want to swap the moves that are important to us (and not sort the entire move vector,
// as that would result in us sorting lots of moves we would never even look at, wasiting lots of time)
void Athena::selectMove(std::vector<MoveData>& moves, Byte startIndex)
{
    for (int i = startIndex + 1; i < moves.size(); i++)
        if (moves[i].moveScore > moves[startIndex].moveScore)
            std::swap(moves[i], moves[startIndex]);
}

void Athena::assignMoveScores(std::vector<MoveData>& moves, Byte ply)
{
    for (int i = 0; i < moves.size(); i++)
    {
        if (moves[i].capturedPieceBB) // move is violent
            moves[i].moveScore += MVV_LVA_OFFSET + MVV_LVATable[getPieceType(moves[i].capturedPieceBB)]
                                                               [getPieceType(moves[i].pieceBB)];
        else // move is quiet
            for (int j = 0; j < MAX_KILLER_MOVES; j++)
                if (moves[i] == mKillerMoves[ply][j])
                {
                    moves[i].moveScore += MVV_LVA_OFFSET - KILLER_MOVE_SCORE;
                    break;
                }
    }
}

void Athena::insertKillerMove(MoveData& move, Byte ply)
{
    if (move == mKillerMoves[ply][0])
        return;
    else
    {
        for (int i = 1; i < MAX_KILLER_MOVES; i++)
            mKillerMoves[ply][i] = mKillerMoves[ply][i - 1];

        mKillerMoves[ply][0] = move;
    }
}

Athena::PieceTypes Athena::getPieceType(Bitboard* pieceBB)
{
    if      (pieceBB == &boardPtr->currentPosition.whitePawnsBB   || pieceBB == &boardPtr->currentPosition.blackPawnsBB)   return PIECE_TYPE_PAWN;
    else if (pieceBB == &boardPtr->currentPosition.whiteKnightsBB || pieceBB == &boardPtr->currentPosition.blackKnightsBB) return PIECE_TYPE_KNIGHT;
    else if (pieceBB == &boardPtr->currentPosition.whiteBishopsBB || pieceBB == &boardPtr->currentPosition.blackBishopsBB) return PIECE_TYPE_BISHOP;
    else if (pieceBB == &boardPtr->currentPosition.whiteRooksBB   || pieceBB == &boardPtr->currentPosition.blackRooksBB)   return PIECE_TYPE_ROOK;
    else if (pieceBB == &boardPtr->currentPosition.whiteQueensBB  || pieceBB == &boardPtr->currentPosition.blackQueensBB)  return PIECE_TYPE_QUEEN;
    else return PIECE_TYPE_KING;
}

int Athena::calculateExtension(Colour side, Byte kingSquare)
{
    // single response


    // check extensions. change this to either king being in check return an extension of + 1? 
    if (boardPtr->squareAttacked(kingSquare, !side))
        return 1;

    return 0;
}

float Athena::getMidgameValue(Bitboard occupiedBB)
{
    int count = 0;
    for (int bit = 0; bit < 64; bit++)
        if ((occupiedBB >> bit) & 1) count++;

    return count/32.f; // 32 = number of total pieces possible
}

void Athena::insertTranspositionEntry(TranspositionHashEntry* hashEntry, int maxEval, int ogAlpha, int beta)
{
    // replacement scheme. let's just replace whenever the depth is greater how about?
    // if we ever get to this point in the search, however, we will replace
    // also, if we were going to search deeper than to whatever depth the transposition went to,
    // we need to disregard it (and just use the best move first)

    // at first, assume that the evaluation recorded in the entry is exact (i.e. derived directly from Athena::evaluatePosition)
    hashEntry->hashFlag = TranspositionHashEntry::EXACT;
     
    // if the maximum evaluation for this transposition was NOT better than move found in a sibling node
    if (maxEval < ogAlpha) hashEntry->hashFlag = TranspositionHashEntry::UPPER_BOUND;
    // if the maximum evaluation for this transposition was too 
    if (maxEval >= beta)   hashEntry->hashFlag = TranspositionHashEntry::LOWER_BOUND;

    mTranspositionTable[hashEntry->zobristKey % mTranspositionTableSize] = *hashEntry;
}

int Athena::getPieceValue(PieceTypes pieceType)
{
    switch (pieceType)
    {
        case PIECE_TYPE_KING:
            return Evaluation::KING_VALUE;
        case PIECE_TYPE_QUEEN:
            return Evaluation::QUEEN_VALUE;
        case PIECE_TYPE_ROOK:
            return Evaluation::ROOK_VALUE;
        case PIECE_TYPE_BISHOP:
            return Evaluation::BISHOP_VALUE;
        case PIECE_TYPE_KNIGHT:
            return Evaluation::KNIGHT_VALUE;
        case PIECE_TYPE_PAWN:
            return Evaluation::PAWN_VALUE;
        default:
            return 0;
    }
}

// note that this function does not currently consider if a move would result in a check
int Athena::see(Byte square, Colour attackingSide, int currentSquareValue)
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
            *pieceBB ^= BB::boardSquares[lsb]; // "make" the move
            int score = -see(square, !attackingSide, pieceValue);
            *pieceBB ^= BB::boardSquares[lsb];// "unmake" the move
            return score;
        }
    }
}

// put in delta pruning
int Athena::quietMoveSearch(Colour side, int alpha, int beta, Byte ply)
{
    mNodes++;
    // the lower bound for the best possible move for the moving side. if no capture move would result in a better position for the playing side,
    // then we just would simply not make the capture move (and return the calculated best move evaluation, aka alpha)
    int standPat = getScoreRelativeToSide(evaluatePosition(getMidgameValue(boardPtr->currentPosition.occupiedBB)), side);

    if (standPat >= beta)
        return beta;

    alpha = std::max(alpha, standPat);

    if (alpha >= beta)
        return beta;

    if (ply >= mMaxPly)
        return alpha;

    boardPtr->calculateSideMovesCapturesOnly(side);
    std::vector<MoveData> moves = boardPtr->getMovesRef(side);
    assignMoveScores(moves, ply);

    for (int i = 0; i < moves.size(); i++)
    {
        selectMove(moves, i);
        if (see(moves[i].targetSquare, side, getPieceValue(getPieceType(moves[i].capturedPieceBB))) < 0)
            continue;

        if (boardPtr->makeMove(&moves[i]))
        {
            int eval = -quietMoveSearch(!side, -beta, -alpha, ply + 1);
            boardPtr->unmakeMove(&moves[i]);

            if (eval >= beta)
                return beta;
            if (eval > alpha)
                alpha = eval;
        }
    }

    return alpha;
}

// alpha is the lower bound for a move's evaluation, beta is the upper bound for a move's evaluation
int Athena::negamax(int depth, Colour side, int alpha, int beta, Byte ply, bool canNullMove)
{
    // OR INSUFFICIENT MATERIAL DRAW
    // simplify it all to an isDraw function
    //if (Outcomes::isThreefoldRepetition(boardPtr->getZobristKeyHistory(), boardPtr->getCurrentPly()) || Outcomes::isFiftyMoveDraw(boardPtr->getFiftyMoveCounter()))
    //    return 0;

    if (depth <= 0)
        return quietMoveSearch(side, alpha, beta, ply);

    // null moves are not allowed if:
    // the side to move being in check
    // the position is in the endgame
    // the previous move was a null move (canNullMove flag)
    // it's the first move of the search
    Byte kingSquare = boardPtr->computeKingSquare(side == SIDE_WHITE ? boardPtr->currentPosition.whiteKingBB : boardPtr->currentPosition.blackKingBB);
    if (canNullMove && getMidgameValue(boardPtr->currentPosition.occupiedBB) > 0.3 && !boardPtr->squareAttacked(kingSquare, !side) && ply != 0)
    {
        // R = 2. hence the - 2
        // notice that we pass in -beta, -beta+1 instead of -beta, -alpha
        // this sets the upper bound to being just 1 greater than the lower bound
        // meaning that any move that is better than the lower bound by just a single point will cause a cutoff
        int eval = -negamax(depth - 1 - 2, !side, -beta, -beta+1, ply + 1, false);
        // if, without making any move, the evaluation comes back and is STILL better than the current worst move, make a cutoff
        if (eval >= beta)
            return eval;
    }

    // used for determining the transposition table entry's flag for this call to negamax
    int ogAlpha = alpha; 

    boardPtr->calculateSideMoves(side);
    std::vector<MoveData> moves = boardPtr->getMovesRef(side);

    int maxEval = -INF;
    assignMoveScores(moves, ply);

    for (int i = 0; i < moves.size(); i++)
    {
        selectMove(moves, i); // swaps current move with the most likely good move in the move list

        // if makemove is legal (i.e. wouldn't result in a check)
        if (boardPtr->makeMove(&moves[i]))
        {
            if (moves[i].moveType == MoveData::EncodingBits::PAWN_PROMOTION)
                boardPtr->promotePiece(&moves[i], MoveData::EncodingBits::QUEEN_PROMO);

            int eval = -negamax(depth - 1 + calculateExtension(side, kingSquare), !side, -beta, -alpha, ply + 1);
            boardPtr->unmakeMove(&moves[i]);

            // checking to see if it's invalid just to ensure that some move is made, even if it is terrible
            if ((eval > maxEval || mMoveToMake.moveType == MoveData::EncodingBits::INVALID) && ply == 0)
            {
                mMoveToMake = moves[i];
                std::cout << "code: " << (int)moves[i].moveType << std::endl;
                mMoveToMake.setMoveType(MoveData::EncodingBits::QUEEN_PROMO);
            }

            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha)
            {
                insertKillerMove(moves[i], ply);
                break;
            }
        }
    }

    // if no moves went through at all (which would result in maxEval == -inf)
    if (maxEval == -INF)
    {
        Byte kingSquare = boardPtr->computeKingSquare(side == SIDE_WHITE ? boardPtr->currentPosition.whiteKingBB : boardPtr->currentPosition.blackKingBB);
        if (boardPtr->squareAttacked(kingSquare, !side)) // checkmate (no legal moves and king is in check)
            return -Evaluation::CHECKMATE_VALUE * depth;
        else
            return 0; // stalemate (no legal moves and king is not in check)
    }

    return alpha;
}
