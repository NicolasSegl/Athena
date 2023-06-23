#pragma once

#include <chrono>
#include <string>
#include <vector>

#include "Bitboard.h"
#include "Board.h"
#include "DataTypes.h"
#include "MoveData.h"
#include "TranspositionHashEntry.h"

// this class defines the engine itself and is how the best move for a given position is found
class Athena
{
private:
    enum PieceTypes
    {
        PIECE_TYPE_KING,
        PIECE_TYPE_QUEEN,
        PIECE_TYPE_ROOK,
        PIECE_TYPE_BISHOP,
        PIECE_TYPE_KNIGHT,
        PIECE_TYPE_PAWN,
        PIECE_TYPE_NONE,
    };

    // the depth to which we should search the move tree
    int mDepth;

    // which colour Athena is playing as 
    Colour mSide;
    
    // holds the best move that Athena could find
    MoveData mMoveToMake;

    // counts the number of nodes that Athena searched
    int mNodes;

    // limits the total number of half-moves that Athena is able to search down to (for performance reasons)
    int mMaxPly;

    // holds the time that athena has left as a in milliseconds
    float mTimeLeft;

    // stores the system time at the very start of the move search
    std::chrono::time_point<std::chrono::steady_clock> mStartTime;

    // reads true if the search is to be halted, reads false otherwise
    bool mHaltSearch;

    // we actually have a pointer to a Board object, as otherwise we'd have to constantly be passing said Board object between functions
    Board* boardPtr;

    // most valuable victim, least valuable attacker table. used for priotizing the
    // moves that would result in the largest material gain
    Byte MVV_LVATable[7][6] =
    {
        { 99, 99, 99, 99, 99, 99},  // Victim: K, Attacker: P, N, B, R, Q, K
        { 20, 19, 19, 18, 17, 16},  // Victim: Q, Attacker: P, N, B, R, Q, K
        { 15, 14, 14, 13, 12, 11},  // Victim: R, Attacker: P, N, B, R, Q, K
        { 10, 9, 9, 8, 7, 6},       // Victim: B, Attacker: P, N, B, R, Q, K
        { 10, 9, 9, 8, 7, 6},       // Victim: N, Attacker: P, N, B, R, Q, K
        { 5,  4, 4, 3, 2, 1},       // Victim: P, Attacker: P, N, B, R, Q, K
        { 0,  0, 0, 0, 0, 0},       // Victim: None, Attacker: P, N, B, R, Q, K
    };
    
    PieceTypes getPieceType(Bitboard* pieceBB);
    int getPieceValue(PieceTypes pieceType);
    int pieceValueTo_MVV_LVA_Index(int value);
    
    // first element is the origin square, second element is the target square
    int mHistoryHeuristic[64][64];

    // holds the list of 2 killer moves for each ply
    MoveData** mKillerMoves;

    void insertKillerMove(MoveData& move, Byte ply);
    
    // points to a large table of transpositions 
    TranspositionHashEntry* mTranspositionTable;

    void clearTranspositionTable();
    void insertTranspositionEntry(ZobristKey::zkey zobristKey, 
								  int bestMoveIndex, 
								  int depth, 
                                  int eval,
								  TranspositionHashEntry::HashFlagValues flag,
                                  Colour side);
    int readTranspositionEntry(ZobristKey::zkey zobristKey, int depth, int alpha, int beta, Colour side);
    
    int negamax
        (
        int depth, 
        Colour side, 
        int alpha, 
        int beta, 
        Byte ply, 
        MoveData* lastMove, 
        bool canNullMove,
        bool isReducedSearch
        );
    int quietMoveSearch(Colour side, int alpha, int beta, Byte ply);

    void assignMoveScores(std::vector<MoveData>& moves, Byte ply, ZobristKey::zkey zkey);
    void selectMove(std::vector<MoveData>& moves, Byte startIndex);
    int calculateExtension(Colour side, Byte kingSquare);

    void checkTimeLeft();
    
public:
    Athena();
    
	MoveData search(Board* board, float timeToMove);
    std::string getOpeningBookMove(Board* board, const std::vector<std::string>& lanStringHistory);

	void setDepth(int newDepth) { mDepth = newDepth; }
    void setColour(Colour side) { mSide = side;      }
    Colour getColour()          { return mSide;      }
};
