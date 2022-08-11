#include "Athena.h"
#include "Outcomes.h"
#include "SquarePieceTables.h"
#include "utils.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>

const int INF = std::numeric_limits<int>::max();

// values of the pieces relative to centipawns (1 pawn is worth 100 centipawns)
const int KING_VALUE = 20000;
const int QUEEN_VALUE = 900;
const int ROOK_VALUE = 500;
const int BISHOP_VALUE = 330;
const int KNIGHT_VALUE = 320;
const int PAWN_VALUE = 100;

// initializes Athena's tranpsosition table and sets the default depth
Athena::Athena()
{   
    mDepth = 6;
    mTranspositionTable = new TranspositionHashEntry[mTranspositionTableSize];
    clearTranspositionTable();
}

// sets all the values in the transposition table to null (so we know that no data has yet been found at a given index)
void Athena::clearTranspositionTable()
{
    for (int i = 0; i < mTranspositionTableSize; i++)
        mTranspositionTable[i].hashFlag = TranspositionHashEntry::NONEXISTENT;
}

// calls negamax and returns the best move that it has found. also outputs some rudimentary data about the search
MoveData Athena::search(Board* board, float timeToMove)
{
    mNodes = 0;
    mMoveToMake.setMoveType(MoveData::EncodingBits::INVALID);
    auto beforeTime = std::chrono::steady_clock::now();
    std::cout << "max eval: " << negamax(board, mDepth, mSide, -INF, INF, 0) << std::endl;
    auto afterTime = std::chrono::steady_clock::now();
    std::cout << "time elapsed: " << std::chrono::duration<double>(afterTime - beforeTime).count() << std::endl;
    return mMoveToMake;
}

std::string Athena::getOpeningBookMove(Board* board, const std::vector<std::string>& lanStringHistory)
{
    std::ifstream bookFile;
    bookFile.open("book.txt");

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
int Athena::evaluatePawnValue(Board* board, int square, Bitboard pawnBB)
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

int Athena::evaluatePosition(Board* board, float midgameValue)
{
    int whiteEval = 0;
    int blackEval = 0;
    
    for (int square = 0; square < 64; square++)
    {
        if (BB::boardSquares[square] & board->currentPosition.emptyBB) // optimization
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
        if (BB::boardSquares[square] & board->currentPosition.whitePawnsBB)
        {
            whiteEval += PAWN_VALUE + pst::pawnTable[63 - square];
        }
        else if (BB::boardSquares[square] & board->currentPosition.whiteKnightsBB) whiteEval += KNIGHT_VALUE + pst::knightTable[63 - square];
        else if (BB::boardSquares[square] & board->currentPosition.whiteBishopsBB) whiteEval += BISHOP_VALUE + pst::bishopTable[63 - square];
        else if (BB::boardSquares[square] & board->currentPosition.whiteRooksBB)   whiteEval += ROOK_VALUE + pst::rookTable[63 - square];
        else if (BB::boardSquares[square] & board->currentPosition.whiteQueensBB)  whiteEval += QUEEN_VALUE + pst::queenTable[63 - square];
        else if (BB::boardSquares[square] & board->currentPosition.whiteKingBB)
            // so a pawn hash table is just a transposition table but for pawn structures??
            whiteEval += KING_VALUE + pst::midgameKingTable[63 - square] * midgameValue + pst::endgameKingTable[63 - square] * (1 - midgameValue);

        else if (BB::boardSquares[square] & board->currentPosition.blackPawnsBB)   blackEval += PAWN_VALUE + pst::pawnTable[square];
        else if (BB::boardSquares[square] & board->currentPosition.blackKnightsBB) blackEval += KNIGHT_VALUE + pst::knightTable[square];
        else if (BB::boardSquares[square] & board->currentPosition.blackBishopsBB) blackEval += BISHOP_VALUE + pst::bishopTable[square];
        else if (BB::boardSquares[square] & board->currentPosition.blackRooksBB)   blackEval += ROOK_VALUE + pst::rookTable[square];
        else if (BB::boardSquares[square] & board->currentPosition.blackQueensBB)  blackEval += QUEEN_VALUE + pst::queenTable[square];
        else if (BB::boardSquares[square] & board->currentPosition.blackKingBB)
            blackEval += KING_VALUE + pst::midgameKingTable[square] * midgameValue + pst::endgameKingTable[square] * (1 - midgameValue);
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

void Athena::assignMoveScores(Board* board, std::vector<MoveData>& moves, Byte ply)
{
    for (int i = 0; i < moves.size(); i++)
    {
        if (moves[i].capturedPieceBB) // move is violent
            moves[i].moveScore += AthenaConstants::MVV_LVA_OFFSET + MVV_LVATable[getPieceType(board, moves[i].capturedPieceBB)]
                                                                              [getPieceType(board, moves[i].pieceBB)];
        else // move is quiet
            for (int j = 0; j < AthenaConstants::MAX_KILLER_MOVES; j++)
                if (moves[i] == mKillerMoves[ply][j])
                {
                    moves[i].moveScore += AthenaConstants::MVV_LVA_OFFSET - AthenaConstants::KILLER_MOVE_SCORE;
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
        for (int i = 1; i < AthenaConstants::MAX_KILLER_MOVES; i++)
            mKillerMoves[ply][i] = mKillerMoves[ply][i - 1];

        mKillerMoves[ply][0] = move;
    }
}

PieceTypes::Types Athena::getPieceType(Board* board, Bitboard* pieceBB)
{
    if      (pieceBB == &board->currentPosition.whitePawnsBB   || pieceBB == &board->currentPosition.blackPawnsBB)   return PieceTypes::PAWN;
    else if (pieceBB == &board->currentPosition.whiteKnightsBB || pieceBB == &board->currentPosition.blackKnightsBB) return PieceTypes::KNIGHT;
    else if (pieceBB == &board->currentPosition.whiteBishopsBB || pieceBB == &board->currentPosition.blackBishopsBB) return PieceTypes::BISHOP;
    else if (pieceBB == &board->currentPosition.whiteRooksBB   || pieceBB == &board->currentPosition.blackRooksBB)   return PieceTypes::ROOK;
    else if (pieceBB == &board->currentPosition.whiteQueensBB  || pieceBB == &board->currentPosition.blackQueensBB)  return PieceTypes::QUEEN;
    else return PieceTypes::KING;
}

int Athena::calculateExtension(Board* board, Colour side, Byte kingSquare)
{
    // single response


    // check extensions. change this to either king being in check return an extension of + 1? 
    if (board->squareAttacked(kingSquare, !side))
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

int Athena::getPieceValue(PieceTypes::Types pieceType)
{
    switch (pieceType)
    {
        case PieceTypes::KING:
            return KING_VALUE;
        case PieceTypes::QUEEN:
            return QUEEN_VALUE;
        case PieceTypes::ROOK:
            return ROOK_VALUE;
        case PieceTypes::BISHOP:
            return BISHOP_VALUE;
        case PieceTypes::KNIGHT:
            return KNIGHT_VALUE;
        case PieceTypes::PAWN:
            return PAWN_VALUE;
        default:
            return 0;
    }
}

// put in delta pruning
int Athena::quietMoveSearch(Board* board, Colour side, int alpha, int beta, Byte ply)
{
    mNodes++;
    // the lower bound for the best possible move for the moving side. if no capture move would result in a better position for the playing side,
    // then we just would simply not make the capture move (and return the calculated best move evaluation, aka alpha)
    int standPat = getScoreRelativeToSide(evaluatePosition(board, getMidgameValue(board->currentPosition.occupiedBB)), side);

    if (standPat >= beta)
        return beta;

    alpha = std::max(alpha, standPat);

    if (alpha >= beta)
        return beta;

    if (ply >= AthenaConstants::MAX_PLY)
        return alpha;

    board->calculateSideMovesCapturesOnly(side);
    std::vector<MoveData> moves = board->getMovesRef(side);
    assignMoveScores(board, moves, ply);

    for (int i = 0; i < moves.size(); i++)
    {
        selectMove(moves, i);

        if (board->makeMove(&moves[i]))
        {
            int eval = -quietMoveSearch(board, !side, -beta, -alpha, ply + 1);
            board->unmakeMove(&moves[i]);

            if (eval >= beta)
                return beta;
            if (eval > alpha)
                alpha = eval;
        }
    }

    return alpha;
}

// alpha is the lower bound for a move's evaluation, beta is the upper bound for a move's evaluation
int Athena::negamax(Board* board, int depth, Colour side, int alpha, int beta, Byte ply, bool canNullMove)
{
    // OR INSUFFICIENT MATERIAL DRAW
    // simplify it all to an isDraw function
    if (Outcomes::isThreefoldRepetition(board->getZobristKeyHistory(), board->getCurrentPly()) || Outcomes::isFiftyMoveDraw(board->getFiftyMoveCounter()))
        return 0;

    if (depth <= 0)
        return quietMoveSearch(board, side, alpha, beta, ply);

    // null moves are not allowed if:
    // the side to move being in check
    // the position is in the endgame
    // the previous move was a null move (canNullMove flag)
    // it's the first move of the search
    Byte kingSquare = board->computeKingSquare(side == SIDE_WHITE ? board->currentPosition.whiteKingBB : board->currentPosition.blackKingBB);
    if (canNullMove && getMidgameValue(board->currentPosition.occupiedBB) > 0.3 && !board->squareAttacked(kingSquare, !side) && ply != 0)
    {
        // R = 2. hence the - 2
        // notice that we pass in -beta, -beta+1 instead of -beta, -alpha
        // this sets the upper bound to being just 1 greater than the lower bound
        // meaning that any move that is better than the lower bound by just a single point will cause a cutoff
        int eval = -negamax(board, depth - 1 - 2, !side, -beta, -beta+1, ply + 1, false);
        // if, without making any move, the evaluation comes back and is STILL better than the current worst move, make a cutoff
        if (eval >= beta)
            return eval;
    }

    // used for determining the transposition table entry's flag for this call to negamax
    int ogAlpha = alpha; 

    board->calculateSideMoves(side);
    std::vector<MoveData> moves = board->getMovesRef(side);

    int maxEval = -INF;
    assignMoveScores(board, moves, ply);

    for (int i = 0; i < moves.size(); i++)
    {
        selectMove(moves, i); // swaps current move with the most likely good move in the move list

        // if makemove is legal (i.e. wouldn't result in a check)
        if (board->makeMove(&moves[i]))
        {
            if (moves[i].moveType == MoveData::EncodingBits::PAWN_PROMOTION)
                board->promotePiece(&moves[i], MoveData::EncodingBits::QUEEN_PROMO);

            int eval = -negamax(board, depth - 1 + calculateExtension(board, side, kingSquare), !side, -beta, -alpha, ply + 1);
            board->unmakeMove(&moves[i]);

            // checking to see if it's invalid just to ensure that some move is made, even if it is terrible
            if ((eval > maxEval || mMoveToMake.moveType == MoveData::EncodingBits::INVALID) && ply == 0)
                mMoveToMake = moves[i];

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
        Byte kingSquare = board->computeKingSquare(side == SIDE_WHITE ? board->currentPosition.whiteKingBB : board->currentPosition.blackKingBB);
        if (board->squareAttacked(kingSquare, !side)) // checkmate (no legal moves and king is in check)
            return -Evaluation::CHECKMATE_SCORE * depth;
        else
            return 0; // stalemate (no legal moves and king is not in check)
    }

    return alpha;
}