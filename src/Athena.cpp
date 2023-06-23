#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>

#include "Athena.h"
#include "Constants.h"
#include "Eval.h"
#include "Outcomes.h"
#include "utils.h"

// represents infinity
const int INF = std::numeric_limits<int>::max();

// move ordering constants
const int MVV_LVA_OFFSET    = 10000000;
const int TT_MOVE_SCORE     = 10000;
const int KILLER_MOVE_SCORE = 10;
const int MAX_KILLER_MOVES  = 2;

const int TRANSPOSITION_TABLE_SIZE = 0x1000000;

const bool CAN_NULL_MOVE    = true;
const bool CANNOT_NULL_MOVE = false;

const int NO_TT_SCORE = -9999999;

// this number defines the number of nodes that will be searched between each check of time
const int TIME_CHECK_INTERVAL = 100;

// initializes Athena's tranpsosition table and sets the default depth
Athena::Athena()
{   
    // default depth of 8 half-moves, with a maximum number of half-moves being searched of 20
    mDepth = 8;
    mMaxPly = 20;

    mTranspositionTable = new TranspositionHashEntry[TRANSPOSITION_TABLE_SIZE];
    clearTranspositionTable();

    // allocates enough memory for two killer moves per ply
    mKillerMoves = new MoveData*[mMaxPly];
    for (int i = 0; i < mMaxPly; i++)
    {
        // initializes the two killer moves we keep track of for each ply (setting them to 0, as no killer moves would have been found yet)
        mKillerMoves[i] = new MoveData[MAX_KILLER_MOVES];
        for (int j = 0; j < 2; j++)
            mKillerMoves[i][j] = {};
    }

    // initializes the history heuristic table (setting all values to 0, as no moves have yet been made that could increase )
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

// calls negamax and returns the best move that it has found
MoveData Athena::search(Board* ptr, float timeToMove)
{
    // initialize values for the upcoming search
    boardPtr    = ptr;
    mNodes      = 0;
    mTimeLeft   = timeToMove;
    mHaltSearch = false;

    // setting this to invalid ensures that if no move was found (due to some sort of bug), there would be no crash, as the move would be considered invalid
    mMoveToMake.moveType = MoveType::INVALID;

    mStartTime = std::chrono::steady_clock::now();
    std::cout << "max eval: " << negamax(mDepth, mSide, -INF, INF, 0, nullptr, CAN_NULL_MOVE, false) << std::endl;
    auto afterTime = std::chrono::steady_clock::now();
    
    // output some rudimentary data about the search
    std::cout << "time elapsed: " << std::chrono::duration<double>(afterTime - mStartTime).count() << std::endl;
    std::cout << "num of nodes: " << mNodes << std::endl;

    return mMoveToMake;
}

/*
   openings books courtesy of lichess. the 5 files are taken from https://github.com/lichess-org/chess-openings
   the .tsv files have the uci codes for the openings at the third tab. from thereon the LAN moves are 
   separated by space

   this function looks into the move history of the game so far, and finds the longest possible opening involving the 
   moves. athena then makes its move according to what the longest opening line says
*/
std::string Athena::getOpeningBookMove(Board* boardPtr, const std::vector<std::string>& lanStringHistory)
{
    std::string moveToMake = "";
    int longestLine = 0;

    // files are labeled a through e
    for (char character = 'a'; character <= 'e'; character++)
    {
        // find book file
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
            splitString(fileLine, fileFields, ASCII::TAB_CODE);

            // get the list of lan string moves for the considered opening
            std::vector<std::string> lanMovesVec;
            splitString(fileFields[3], lanMovesVec, ' ');

            // if the number of moves in the opening found is greater than our currently considered opening, then we might
            // have found a possible opening we can use instead
            if (lanMovesVec.size() > longestLine && lanMovesVec.size() > lanStringHistory.size())
                for (int move = 0; move < lanMovesVec.size(); move++)
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
    
    return moveToMake;
}

// decides whether or not a move will be inserted into the transposition table
// in essence, it checks whether or not the move that was just considered by Athena was more or less accurate than the current
// entry in the index given by the zobrist key. if it is less accurate, no replacement occurs.
void Athena::insertTranspositionEntry(ZobristKey::zkey zobristKey, 
									  int bestMoveIndex, 
									  int depth, 
									  int eval, 
									  TranspositionHashEntry::HashFlagValues flag,
                                      Colour side)
{
    // fetch the entry currently in the table by the given zobrist key
    TranspositionHashEntry* currentEntry = &mTranspositionTable[zobristKey % TRANSPOSITION_TABLE_SIZE];

    if (currentEntry->depth > depth)
        return;

    currentEntry->depth = depth;
    currentEntry->eval = eval;
    currentEntry->hashFlag = flag;
    currentEntry->bestMoveIndex = bestMoveIndex;
    currentEntry->zobristKey = zobristKey;
    currentEntry->side = side;
}

// reads the data from the transposition table given the zobrist key's hash value
// if no such entry exists yet, then a value is returned indicating that no entry could be found
int Athena::readTranspositionEntry(ZobristKey::zkey zobristKey, int depth, int alpha, int beta, Colour side)
{
	TranspositionHashEntry* hashEntry = &mTranspositionTable[zobristKey % TRANSPOSITION_TABLE_SIZE];
	if (hashEntry->zobristKey == zobristKey && hashEntry->depth >= depth)
	{
        if (hashEntry->side != side)
            return NO_TT_SCORE;

        if (hashEntry->hashFlag == TranspositionHashEntry::EXACT)
            return hashEntry->eval;
		else if (hashEntry->hashFlag == TranspositionHashEntry::UPPER_BOUND && hashEntry->eval <= alpha)
			return alpha;
		else if (hashEntry->hashFlag == TranspositionHashEntry::LOWER_BOUND && hashEntry->eval >= beta)
			return beta;
	}

	return NO_TT_SCORE;
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

// converts the type of the piece into the value that can be used for indexing the MVV_LVA table
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

// gives moves weight values based on various factors. the higher the weight value, the earlier we should search that 
// move, as as higher value indicates that the move might be better than another move with a lower value
void Athena::assignMoveScores(std::vector<MoveData>& moves, Byte ply, ZobristKey::zkey zkey)
{
    for (int i = 0; i < moves.size(); i++)
    {
        // if the move is violent (i.e. involves a piece being captured), then assign a move score based on
        // the attacking piece's type and the victim piece's type
        if (moves[i].capturedPieceBB)
            moves[i].moveScore += MVV_LVA_OFFSET + MVV_LVATable[pieceValueTo_MVV_LVA_Index(moves[i].capturedPieceValue)]
                                                               [pieceValueTo_MVV_LVA_Index(moves[i].pieceValue)]; // make a function to convert 1, 3, 5, 9 to index?
        else // otherwise, if the move is quiet (no piece being captured)
        {
            // check to see if the move was a killer move in a previous search (that is, check to see if the move
            // caused a significant advantage for the side moving)
            for (int j = 0; j < MAX_KILLER_MOVES; j++)
                if (moves[i] == mKillerMoves[ply][j])
                {
                    moves[i].moveScore += MVV_LVA_OFFSET - KILLER_MOVE_SCORE;
                    break;
                }

            // add the weight of the history of that move (i.e., has this particular origin square and target square ever
            // given an advantage to the side that made the move?)
            moves[i].moveScore += mHistoryHeuristic[moves[i].originSquare][moves[i].targetSquare];
        }
    }
}

// shifts the oldest killer move searched off the end of the table, putting the younger killer move in its place
// then, the killer move that is being inserted is placed in the first index of the killer move table
void Athena::insertKillerMove(MoveData& move, Byte ply)
{
    // do not insert the killer move if the move is already in the killer move table
    if (move == mKillerMoves[ply][0])
        return;
    else
    {
        for (int i = 1; i < MAX_KILLER_MOVES; i++)
            mKillerMoves[ply][i] = mKillerMoves[ply][i - 1];

        mKillerMoves[ply][0] = move;
    }
}

// returns the type of piece a piece is, based on the piece bitboard to which it belongs
Athena::PieceTypes Athena::getPieceType(Bitboard* pieceBB)
{
    if      (pieceBB == &boardPtr->currentPosition.whitePawnsBB   || pieceBB == &boardPtr->currentPosition.blackPawnsBB)   return PIECE_TYPE_PAWN;
    else if (pieceBB == &boardPtr->currentPosition.whiteKnightsBB || pieceBB == &boardPtr->currentPosition.blackKnightsBB) return PIECE_TYPE_KNIGHT;
    else if (pieceBB == &boardPtr->currentPosition.whiteBishopsBB || pieceBB == &boardPtr->currentPosition.blackBishopsBB) return PIECE_TYPE_BISHOP;
    else if (pieceBB == &boardPtr->currentPosition.whiteRooksBB   || pieceBB == &boardPtr->currentPosition.blackRooksBB)   return PIECE_TYPE_ROOK;
    else if (pieceBB == &boardPtr->currentPosition.whiteQueensBB  || pieceBB == &boardPtr->currentPosition.blackQueensBB)  return PIECE_TYPE_QUEEN;
    else return PIECE_TYPE_KING;
}

// calculates by how many extra plys we should search the move
int Athena::calculateExtension(Colour side, Byte kingSquare)
{
    // if the king is attacked, then we should search 1 extra ply deep
    if (boardPtr->squareAttacked(kingSquare, !side))
        return 1;

    return 0;
}

// returns the centipawn value of a given piece
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

// this is an implentation of quiescence serach. it looks at all the violent moves (i.e., moves that involve a capture)
// and evaluates them slightly further than the default depth, as to prevent the horizon problem
int Athena::quietMoveSearch(Colour side, int alpha, int beta, Byte ply)
{
    mNodes++;

    // represents as a decimal how far into the midgame we are. A value of 1.0 indicates the start, and a value of 0.0 would represent endgame
    float midgameValue = Eval::getMidgameValue(boardPtr->currentPosition.occupiedBB);

    // the lower bound for the best possible move for the moving side. if no capture move would result in a better position for the playing side,
    // then we just would simply not make the capture move (and return the calculated best move evaluation, aka alpha)
    int standPat = Eval::evaluateBoardRelativeTo(side, Eval::evaluatePosition(boardPtr, midgameValue));
    if (standPat >= beta)
        return beta;

    alpha = std::max(alpha, standPat);

    // alpha beta pruning
    if (alpha >= beta)
        return beta;

    // halt the search if we begin to search past the maximum number of plys
    if (ply >= mMaxPly)
        return alpha;

    // populate a vector with the violent moves for the side to play
    std::vector<MoveData> moves;
    MoveGeneration::calculateSideMoves(boardPtr, side, moves, true);

    // assign priority to the moves in the vector
    ZobristKey::zkey positionZKey = boardPtr->getZobristKeyHistory()[boardPtr->getCurrentPly()];
    assignMoveScores(moves, ply, positionZKey);

    for (int i = 0; i < moves.size(); i++)
    {
        selectMove(moves, i);

        // delta pruning
        // essentially, it will cast away a move if it determines that it's value isn't significant enough
        if (moves[i].capturedPieceValue + 200 < alpha && midgameValue > 0.25)
            continue;

        // if the static search evaluation of the square being attacked is less than 0 (indicating that the side to move would lose
        // material if it made the move), then we cast the move away and move on to the next violent move
        if (Eval::see(boardPtr, moves[i].targetSquare, side, moves[i].capturedPieceValue) < 0)
            continue;

        // similar process as to that which occurs in minimax. searches all the possible children nodes and determines which move is best
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

// halts the move search if Athena has been using too much time (as to prevent timeout)
void Athena::checkTimeLeft()
{
    static int nodeCounter = 0;
    nodeCounter++;

    // check to see if Athena has taken too much time every so many nodes (as defined by TIME_CHECK_INTERVAL)
    if (nodeCounter >= TIME_CHECK_INTERVAL)
    {
        // reset the node counter
        nodeCounter = 0;
        
        // if the current move has taken up 5% or more of the remainder of Athena's time, then we will simply use whichever move we have found and halt the search
        if (std::chrono::duration<double>(std::chrono::steady_clock::now() - mStartTime).count() * 1000 >= 0.05 * mTimeLeft)
            mHaltSearch = true;
    }
}

/*
    this is an implentation of the minimax algorithm. negamax is simply an easy method to reduce the sheer number of lines of repeated code necessary
    it is the primary function of the engine, as it searches through all the moves and evaluates their worth
    alpha is the lower bound for a move's evaluation, beta is the upper bound for a move's evaluation
*/

int Athena::negamax(int depth, Colour side, int alpha, int beta, Byte ply, MoveData* lastMove, bool canNullMove, bool isReducedSearch)
{
    // immediately return if the search has been halted
    if (mHaltSearch)
       return 0;

    ZobristKey::zkey positionZKey = boardPtr->getZobristKeyHistory()[boardPtr->getCurrentPly()];

    int ttScore = readTranspositionEntry(positionZKey, depth, alpha, beta, side);
    TranspositionHashEntry::HashFlagValues hashFlag = TranspositionHashEntry::HashFlagValues::UPPER_BOUND;
    int isPvNode = beta - alpha > 1;

    if (ttScore != NO_TT_SCORE && depth != mDepth && isPvNode == 0)
       return ttScore;

    // return an evaluation of 0 if a draw occured
    if (Outcomes::isDraw(boardPtr))
        return 0;

    // this if statement checks if we should return an evaluation based on the current board position, or 
    // if we must search a little deeper to see if a dangerous move is lurking around the horizon for the side to play
    if (depth <= 0)
    {
        if (lastMove)
            if (lastMove->capturedPieceBB) // this indicates a violent move. it means that we should search until we encounter only quiet (non-capture) moves
                return quietMoveSearch(side, alpha, beta, ply);

        // if the last move was not a capturing move, then we simply need to return the 
        // evaluation of the current position, relative to the side that is playing
        float midgameValue = Eval::getMidgameValue(boardPtr->currentPosition.occupiedBB);
        return Eval::evaluateBoardRelativeTo(side, Eval::evaluatePosition(boardPtr, midgameValue));
    }
    else
    {
        checkTimeLeft();
        mNodes++;
    }

    Bitboard kingBB = side == SIDE_WHITE ? boardPtr->currentPosition.whiteKingBB : boardPtr->currentPosition.blackKingBB;
    Byte kingSquare = boardPtr->computeKingSquare(kingBB);
    bool inCheck = boardPtr->squareAttacked(kingSquare, !side);
        
    // if for whatever reason the side to play has no king, return a huge negative value
    if (!kingBB)
        return -100000;
    
    /*
        make a null move if possible.
        null moves are not allowed if:
            the side to move being in check
            the position is in the endgame
            the previous move was a null move (canNullMove flag)
            it's the first move of the search
    */
    if (canNullMove && Eval::getMidgameValue(boardPtr->currentPosition.occupiedBB) > 0.3 && !inCheck && ply != 0)
    {
        // R = 2. hence the - 2 in (depth - 1 - 2)
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
	
    // populate a vector with the moves for the side to play
    std::vector<MoveData> moves;
    MoveGeneration::calculateSideMoves(boardPtr, side, moves, false);

    // assign priorty (a value that determines how early the move should be searched) to the moves in the move vector
    assignMoveScores(moves, ply, positionZKey);

    // assign an infinitely small value to the maximum evalation (so that any move would increase it)
    int maxEval = -INF;

    bool foundPVMove = false;
    for (int i = 0; i < moves.size(); i++)
    {
        selectMove(moves, i); // swaps current move with the most likely good move in the move list

        // if the move is legal (i.e. wouldn't result in a check)
        if (boardPtr->makeMove(&moves[i]))
        {
            // if a pawn can be promoted, always assume a queen promotion for simplicity sake
            if (moves[i].moveType == MoveType::PAWN_PROMOTION)
                boardPtr->promotePiece(&moves[i], MoveType::QUEEN_PROMO);

            /*
            Principial Variation Search (pvs):
                fully search minimax after we've found a move that has improved alpha (i.e. a candidate for the best move, the PV move)
                after that, only search minimax in a restricted a/b window
            */
            int eval;
            if (!foundPVMove)
                eval = -negamax(depth - 1 + calculateExtension(side, kingSquare), !side, -beta, -alpha, ply + 1, &moves[i], CAN_NULL_MOVE, isReducedSearch);
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
                    eval = -negamax(depth - 1 + calculateExtension(side, kingSquare), !side, -beta, -alpha, ply + 1, &moves[i], CAN_NULL_MOVE, isReducedSearch);
            }

            // unmake the move as to assume the board position prior to the move
            boardPtr->unmakeMove(&moves[i]);

            // this ensures that Athena always make a move (mostly just used as a failsafe)
            if ((eval > maxEval || mMoveToMake.moveType == MoveType::INVALID) && ply == 0)
                mMoveToMake = moves[i];

            // immediately break out of the move loop if the search has been halted
            if (mHaltSearch)
                break;

            // should the move just tested be the best move so far, set the maxmimum evaluation to its evaluation and set the best move index
            // to the current index (so that the transposition table can be used for sorting move priorities)
			if (eval > maxEval)
            {
				maxEval = eval;
                bestMoveIndex = i;
            }

            // checks to see if this move is better than the previosuly thought best move for this turn
            if (eval > alpha)
            {
                alpha = eval;
                foundPVMove = true;
                hashFlag = TranspositionHashEntry::HashFlagValues::EXACT;

                // update the history heuristic table for future move prioritizing if the move is quiet (i.e. a non-capture move)
                if (!moves[i].capturedPieceBB)
                    mHistoryHeuristic[moves[i].originSquare][moves[i].targetSquare] += depth * depth;
            }

            // this is a beta cutoff. it pretty much says that if this move is so good that the other side would never allow it,
            // then we shouldn't bother searching any farther
            if (beta <= eval)
            {
                insertTranspositionEntry(positionZKey, bestMoveIndex, depth, beta, TranspositionHashEntry::HashFlagValues::LOWER_BOUND, side);
                
                // if the move was quiet, insert it into the killer move table. this will allow for better move prioritizing in 
                // future searches (as it will know to assign this move a higher weight, even though it is seemingly not an extraordinary move)
                if (!moves[i].capturedPieceBB)
                    insertKillerMove(moves[i], ply);

                return beta;
            }
        }
    }

    // checks to see if no moves went through at all (which means that there were no legal moves)
    if (maxEval == -INF)
    {
        // check to see if the king is attacked (which, if it had no legal moves, would indicate checkmate)
        Byte kingSquare = boardPtr->computeKingSquare(side == SIDE_WHITE ? boardPtr->currentPosition.whiteKingBB : boardPtr->currentPosition.blackKingBB);
        if (boardPtr->squareAttacked(kingSquare, !side))
            return -Eval::CHECKMATE_VALUE * depth;

        // otherwise, if there are no legal moves but the king is not in check, then return a value of 0, as a stalemate would have occured (draw)
        else
            return 0;
    }	

    insertTranspositionEntry(positionZKey, bestMoveIndex, depth, alpha, hashFlag, side);

    return alpha;
}
