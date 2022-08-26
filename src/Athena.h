#pragma once

#include <string>
#include <vector>

#include "Bitboard.h"
#include "Board.h"
#include "DataTypes.h"
#include "MoveData.h"
#include "TranspositionHashEntry.h"

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
    
    MoveData** mKillerMoves;
    void insertKillerMove(MoveData& move, Byte ply);

    int mTranspositionTableSize = 20000000;
    TranspositionHashEntry* mTranspositionTable;
    void clearTranspositionTable();
    void insertTranspositionEntry(TranspositionHashEntry* hashEntry, int maxEval, int ogAlpha, int beta);
    
	int mDepth;
    Colour mSide;
    MoveData mMoveToMake;

    int mNodes;
    int mMaxPly;
    Board* boardPtr;
    
    int negamax(int depth, Colour side, int alpha, int beta, Byte ply, bool canNullMove);
    int quietMoveSearch(Colour side, int alpha, int beta, Byte ply);

    void assignMoveScores(std::vector<MoveData>& moves, Byte ply);
    void selectMove(std::vector<MoveData>& moves, Byte startIndex);
    int calculateExtension(Colour side, Byte kingSquare);
    
public:
    Athena();
    
	MoveData search(Board* board, float timeToMove);
    std::string getOpeningBookMove(Board* board, const std::vector<std::string>& lanStringHistory);

	void setDepth(int newDepth) { mDepth = newDepth; }
    void setColour(Colour side) { mSide = side;      }
    Colour getColour()          { return mSide;      }
};
