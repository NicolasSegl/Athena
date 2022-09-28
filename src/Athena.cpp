#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>

#include "Athena.h"
#include "Constants.h"
#include "Eval.h"
#include "Outcomes.h"
#include "utils.h"

const int INF = std::numeric_limits<int>::max();

// move ordering constants
const int MVV_LVA_OFFSET = 10000000;
const int TT_MOVE_SCORE = 10000;
const int KILLER_MOVE_SCORE = 10;
const int MAX_KILLER_MOVES = 2;

const int TRANSPOSITION_TABLE_SIZE = 1000000;

const bool CAN_NULL_MOVE    = true;
const bool CANNOT_NULL_MOVE = false;

// initializes Athena's tranpsosition table and sets the default depth
Athena::Athena()
{   
    mDepth = 10;
    mMaxPly = 20;
    mTranspositionTable = new TranspositionHashEntry[TRANSPOSITION_TABLE_SIZE];
    clearTranspositionTable();

    mKillerMoves = new MoveData*[mMaxPly];
    for (int i = 0; i < mMaxPly; i++)
    {
        mKillerMoves[i] = new MoveData[MAX_KILLER_MOVES];
        for (int j = 0; j < 2; j++)
            mKillerMoves[i][j] = {};
    }
    for (int i = 0; i < 64; i++)
        for (int j = 0; j < 64; j++)
            mHistoryHeuristic[i][j] = 0;
}

// sets all the values in the transposition table to null (so we know that no data has yet been found at a given index)
void Athena::clearTranspositionTable()
{
    for (int i = 0; i < TRANSPOSITION_TABLE_SIZE; i++)
        mTranspositionTable[i].hashFlag = TranspositionHashEntry::NONEXISTENT;
}

// calls negamax and returns the best move that it has found. also outputs some rudimentary data about the search
MoveData Athena::search(Board* ptr, float timeToMove)
{
    boardPtr = ptr;
    mNodes = 0;
    mMoveToMake.setMoveType(MoveData::EncodingBits::INVALID);

    auto beforeTime = std::chrono::steady_clock::now();
    std::cout << "max eval: " << negamax(mDepth, mSide, -INF, INF, 0, nullptr, CAN_NULL_MOVE, false) << std::endl;
    auto afterTime = std::chrono::steady_clock::now();
    std::cout << "time elapsed: " << std::chrono::duration<double>(afterTime - beforeTime).count() << std::endl;
    std::cout << "num of nodes: " << mNodes << std::endl;

    return mMoveToMake;
}

/*
   openings books courtesy of lichess. the 5 files are taken from https://github.com/lichess-org/chess-openings
   the .tsv files have the uci codes for the openings at the third tab. from thereon the LAN moves are 
   separated by space
*/
std::string Athena::getOpeningBookMove(Board* boardPtr, const std::vector<std::string>& lanStringHistory)
{
    // files are labeled a through e

    std::string moveToMake = "";
    int longestLine = 0;
    for (char character = 'a'; character <= 'e'; character++)
    {
        std::ifstream bookFile;
        std::string fileName = "books/";
        fileName += character;
        fileName += ".tsv";
        bookFile.open(fileName);

        // extract the data from the opening book files
        std::vector<std::string> fileFields;
        std::string fileLine;
        while (std::getline(bookFile, fileLine))
        {
            fileFields.clear();
            //std::getline(bookFile, fileLine);
            splitString(fileLine, fileFields, ASCII::TAB_CODE); // 0x9 is the ascii code character for tabs

            // get the list of lan string moves for the considered opening
            std::vector<std::string> lanMovesVec;
            splitString(fileFields[3], lanMovesVec, ' ');

            for (int move = 0; move < lanMovesVec.size(); move++)
            {
                if (lanMovesVec.size() > longestLine && lanMovesVec.size() > lanStringHistory.size())
                {
                    // if the moves made so far in the game match up with this opening line's moves
                    bool isActiveLine = true;
                    for (int move = 0; move < lanStringHistory.size(); move++)
                        if (lanStringHistory[move] != lanMovesVec[move])
                        {
                            isActiveLine = false;
                            break;
                        }

                    // then set the next as the move to make
                    if (isActiveLine)
                    {
                        moveToMake = lanMovesVec[lanStringHistory.size()]; // indexes the current opening line move plus 1
                        longestLine = lanMovesVec.size();
                    }
                }
            }
        }
    }
    
    return moveToMake;
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

int Athena::pieceValueTo_MVV_LVA_Index(int value)
{
    switch (value)
    {
    case Eval::PAWN_VALUE:
        return PIECE_TYPE_PAWN;
    case Eval::KNIGHT_VALUE:
        return PIECE_TYPE_KNIGHT;
    case Eval::BISHOP_VALUE:
        return PIECE_TYPE_BISHOP;
    case Eval::ROOK_VALUE:
        return PIECE_TYPE_ROOK;
    case Eval::QUEEN_VALUE:
        return PIECE_TYPE_QUEEN;
    default:
        return PIECE_TYPE_KING;
    }
}

void Athena::assignMoveScores(std::vector<MoveData>& moves, Byte ply, ZobristKey::zkey zkey)
{
    // order the move from the transposition table (if it exists) above all other moves
    int tableIndex = zkey % TRANSPOSITION_TABLE_SIZE;
    if (mTranspositionTable[tableIndex].zobristKey == zkey && 
        mTranspositionTable[tableIndex].bestMoveIndex >= 0)
        {
           // moves[mTranspositionTable[tableIndex].bestMoveIndex].moveScore += MVV_LVA_OFFSET + TT_MOVE_SCORE;
        }
    
    for (int i = 0; i < moves.size(); i++)
    {
        if (moves[i].capturedPieceBB) // move is violent
            moves[i].moveScore += MVV_LVA_OFFSET + MVV_LVATable[pieceValueTo_MVV_LVA_Index(moves[i].capturedPieceValue)]
                                                               [pieceValueTo_MVV_LVA_Index(moves[i].pieceValue)]; // make a function to convert 1, 3, 5, 9 to index?
        else // move is quiet
        {
            for (int j = 0; j < MAX_KILLER_MOVES; j++)
                if (moves[i] == mKillerMoves[ply][j])
                {
                    moves[i].moveScore += MVV_LVA_OFFSET - KILLER_MOVE_SCORE;
                    break;
                }

            moves[i].moveScore += mHistoryHeuristic[moves[i].originSquare][moves[i].targetSquare];
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

// the zobrist key should be considering who is moving?
void Athena::insertTranspositionEntry(ZobristKey::zkey zobristKey, 
									  int bestMoveIndex, 
									  Byte depth, 
									  short eval, 
									  int beta, 
									  int ogAlpha)
{
    TranspositionHashEntry* currentEntry = &mTranspositionTable[zobristKey % TRANSPOSITION_TABLE_SIZE];

    /*
        if there exists a position at this index already, we need to see if we should replace it
        if our current entry has searched further, then we will replace the current table entry
    */
  //  if (currentEntry->zobristKey)
    {
		// the higher the depth, the further the node has been searched
     //   if (currentEntry->depth > depth)
	//		return; // if not too old, then return (i.e. do not replace)
    }

	// now replace the data in the current entry

    // at first, assume that the evaluation recorded in the entry is exact (i.e. derived directly from Athena::evaluatePosition)
    currentEntry->hashFlag = TranspositionHashEntry::EXACT;
     
    // if the maximum evaluation for this transposition was NOT better than move found in a sibling node
    if (eval < ogAlpha)
	{
		 currentEntry->hashFlag = TranspositionHashEntry::UPPER_BOUND;
		 eval = ogAlpha;
	}
    // if the maximum evaluation for this transposition was too 
    if (eval >= beta) 
	{
		currentEntry->hashFlag = TranspositionHashEntry::LOWER_BOUND;
		eval = beta;
	}

	currentEntry->depth = depth;
	currentEntry->zobristKey = zobristKey;

	currentEntry->bestMoveIndex = bestMoveIndex;
}

// when reading data from the TT, we need to compare zkeys
// also, replace based on age AND depth
const int NO_TT_SCORE = -9999999;
int Athena::readTranspositionEntry(ZobristKey::zkey zobristKey, int depth, int alpha, int beta)
{
	TranspositionHashEntry* hashEntry = &mTranspositionTable[zobristKey % TRANSPOSITION_TABLE_SIZE];
	if (hashEntry->zobristKey == zobristKey && hashEntry->depth >= depth)
	{
		if (hashEntry->hashFlag == TranspositionHashEntry::UPPER_BOUND && hashEntry->eval <= alpha)
			return alpha;
		else if (hashEntry->hashFlag == TranspositionHashEntry::LOWER_BOUND && hashEntry->eval >= beta)
			return beta;
		else // exact value
			return hashEntry->eval;
	}

	return NO_TT_SCORE;
}

int Athena::getPieceValue(PieceTypes pieceType)
{
    switch (pieceType)
    {
        case PIECE_TYPE_KING:
            return Eval::KING_VALUE;
        case PIECE_TYPE_QUEEN:
            return Eval::QUEEN_VALUE;
        case PIECE_TYPE_ROOK:
            return Eval::ROOK_VALUE;
        case PIECE_TYPE_BISHOP:
            return Eval::BISHOP_VALUE;
        case PIECE_TYPE_KNIGHT:
            return Eval::KNIGHT_VALUE;
        case PIECE_TYPE_PAWN:
            return Eval::PAWN_VALUE;
        default:
            return 0;
    }
}

// put in delta pruning
int Athena::quietMoveSearch(Colour side, int alpha, int beta, Byte ply)
{
    mNodes++;
    // the lower bound for the best possible move for the moving side. if no capture move would result in a better position for the playing side,
    // then we just would simply not make the capture move (and return the calculated best move evaluation, aka alpha)
    float midgameValue = Eval::getMidgameValue(boardPtr->currentPosition.occupiedBB);
    int standPat = Eval::evaluateBoardRelativeTo(side, Eval::evaluatePosition(boardPtr, midgameValue));

    if (standPat >= beta)
        return beta;

    alpha = std::max(alpha, standPat);

    if (alpha >= beta)
        return beta;

    if (ply >= mMaxPly)
        return alpha;

    std::vector<MoveData> moves;
    MoveGeneration::calculateSideMoves(boardPtr, side, moves, true);

    ZobristKey::zkey positionZKey = boardPtr->getZobristKeyHistory()[boardPtr->getCurrentPly()];
    assignMoveScores(moves, ply, positionZKey);

    for (int i = 0; i < moves.size(); i++)
    {
        selectMove(moves, i);
        if (moves[i].capturedPieceValue + 200 < alpha && midgameValue > 0.25)
            continue;
        if (Eval::see(boardPtr, moves[i].targetSquare, side, moves[i].capturedPieceValue) < 0)
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
int Athena::negamax(int depth, Colour side, int alpha, int beta, Byte ply, MoveData* lastMove, bool canNullMove, bool isReducedSearch)
{
	// since the zobrist key is the same as it was when we exited. wait no, we should have made the move?
	// is the zobrist key just not getting updated?
	ZobristKey::zkey positionZKey = boardPtr->getZobristKeyHistory()[boardPtr->getCurrentPly()];
	int ttScore = readTranspositionEntry(positionZKey, depth, alpha, beta);
	if (ttScore != NO_TT_SCORE)
        return ttScore;

    // OR INSUFFICIENT MATERIAL DRAW
    if (Outcomes::isDraw(boardPtr))
        return 0;

    if (depth <= 0)
    {
        if (lastMove)
            if (lastMove->capturedPieceBB)
                return quietMoveSearch(side, alpha, beta, ply);

        float midgameValue = Eval::getMidgameValue(boardPtr->currentPosition.occupiedBB);
        return Eval::evaluateBoardRelativeTo(side, Eval::evaluatePosition(boardPtr, midgameValue));
    }

    Bitboard kingBB = side == SIDE_WHITE ? boardPtr->currentPosition.whiteKingBB : boardPtr->currentPosition.blackKingBB;
    Byte kingSquare = boardPtr->computeKingSquare(kingBB);
    bool inCheck = boardPtr->squareAttacked(kingSquare, !side);

    // futility pruning
    //if (depth == 1)
      //  if (canNullMove && !inCheck && )
        
    // if for whatever reason the side to play has no king, return a huge negative value
    if (!kingBB)
        return -100000;
    
    /*
     null moves are not allowed if:
        the side to move being in check
        the position is in the endgame
        the previous move was a null move (canNullMove flag)
        it's the first move of the search
    */
    if (canNullMove && Eval::getMidgameValue(boardPtr->currentPosition.occupiedBB) > 0.3 && !inCheck && ply != 0)
    {
        // R = 2. hence the - 2
        // notice that we pass in -beta, -beta+1 instead of -beta, -alpha
        // this sets the upper bound to being just 1 greater than the lower bound
        // meaning that any move that is better than the lower bound by just a single point will cause a cutoff
        int eval = -negamax(depth - 1 - 2, !side, -beta, -beta+1, ply + 1, lastMove, false, true);
        // if, without making any move, the evaluation comes back and is STILL better than the current worst move, make a cutoff
        if (eval >= beta)
            return eval;
    }

    // used for determining the transposition table entry's flag for this call to negamax
    int ogAlpha = alpha; 
	
	int bestMoveIndex = -1;

    std::vector<MoveData> moves;
    MoveGeneration::calculateSideMoves(boardPtr, side, moves, false);

    int maxEval = -INF;
    assignMoveScores(moves, ply, positionZKey);

    bool foundPVMove = false;
    for (int i = 0; i < moves.size(); i++)
    {
        selectMove(moves, i); // swaps current move with the most likely good move in the move list
        
        // if makemove is legal (i.e. wouldn't result in a check)
        if (boardPtr->makeMove(&moves[i]))
        {
            if (moves[i].moveType == MoveData::EncodingBits::PAWN_PROMOTION)
                boardPtr->promotePiece(&moves[i], MoveData::EncodingBits::QUEEN_PROMO);

            /*
            Principial Variation Search (pvs):
                fully search minimax after we've found a move that has improved alpha (i.e. a candidate for the best move, the PV move)
                after that, only search minimax in a restricted a/b window
            */
            int eval;
            if (!foundPVMove)
                eval = -negamax(depth - 1 + calculateExtension(side, kingSquare), !side, -beta, -alpha, ply + 1, &moves[i], CAN_NULL_MOVE, false);
            else
            {
                /*
                if we have our PV move (i.e.the move that has improved alpha and that we are assuming to be the best move possible):
                    search through minimax with a null move, seeing if it is at all possible for alpha to be increased even a little
                    if it is possible (the evaluation is greater than our current alpha), then research the whole tree to find the new
                    best move (PV move)
                */
                eval = -negamax(depth - 1 + calculateExtension(side, kingSquare), !side, -alpha - 1, -alpha, ply + 1, &moves[i], CAN_NULL_MOVE, true);
                if (eval > alpha)
                    eval = -negamax(depth - 1 + calculateExtension(side, kingSquare), !side, -beta, -alpha, ply + 1, &moves[i], CAN_NULL_MOVE, false);
            }

            boardPtr->unmakeMove(&moves[i]);

            // checking to see if it's invalid just to ensure that some move is made, even if it is terrible
			// change this to a pointer to the best move. then at the end, if ply == 0, we can set it to mMoveToMake
            if ((eval > maxEval || mMoveToMake.moveType == MoveData::EncodingBits::INVALID) && ply == 0)
                mMoveToMake = moves[i];

			if (eval > maxEval)
			{
				maxEval = eval;
				bestMoveIndex = i;
			}

            if (eval > alpha)
            {
                alpha = eval;
                foundPVMove = true;
                if (!moves[i].capturedPieceBB)
                    mHistoryHeuristic[moves[i].originSquare][moves[i].targetSquare] += depth * depth;
            }

            if (beta <= alpha)
            {
                // if quiet move
                if (!moves[i].capturedPieceBB)
                    insertKillerMove(moves[i], ply);

                break;
            }
        }
    }

    if (!isReducedSearch)
	    insertTranspositionEntry(positionZKey, bestMoveIndex, depth, maxEval, beta, ogAlpha);

    // if no moves went through at all (which would result in maxEval == -inf)
    if (maxEval == -INF)
    {
        Byte kingSquare = boardPtr->computeKingSquare(side == SIDE_WHITE ? boardPtr->currentPosition.whiteKingBB : boardPtr->currentPosition.blackKingBB);
        if (boardPtr->squareAttacked(kingSquare, !side)) // checkmate (no legal moves and king is in check)
            return -Eval::CHECKMATE_VALUE * depth;
        else
            return 0; // stalemate (no legal moves and king is not in check)
    }	

    return alpha;
}
