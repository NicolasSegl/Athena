#pragma once

#include <vector>
#include <string>

#include "Bitboard.h"
#include "MoveGenerator.h"
#include "MoveData.h"
#include "ChessPosition.h"
#include "ZobristKeyGenerator.h"

class Board
{    
private:
	std::vector<MoveData> mWhiteMoves;
	std::vector<MoveData> mBlackMoves;

    MoveGenerator mMoveGenerator;

	uint64_t mCurrentZobristKey;
	ZobristKeyGenerator mZobristKeyGenerator;
	ZobristKey mZobristKeyHistory[1000];

	short mPly;
	short mFiftyMoveCounter;

	void setBitboards();

	// functions for making/unmaking moves
	void setCastleMoveData(MoveData* castleMoveData, MoveData* kingMD, MoveData* rookMD);
	bool makeCastleMove(MoveData* moveData);
	void setEnPassantSquares(MoveData* moveData);
	void updateBitboardWithMove(MoveData* moveData);
	void undoPromotion(MoveData* moveData);

	// zobrist keys
	void insertMoveIntoHistory();
	void deleteMoveFromHistory();

	// fen strings
	void setFENPiecePlacement(char pieceType, Byte square);
	std::string getSquareStringCoordinate(Byte square);
	Byte getSquareNumberCoordinate(const std::string& stringCoordinate);

public:
	Board() {}

	ChessPosition currentPosition;
	Byte endgameValue, midgameValue;

	void init();

	// functions interpretting different chess notations (mainly for uci/opening book usage)
	std::string getMoveLANString(MoveData* moveData);
	bool makeMoveLAN(const std::string& lanString);
	void setPositionFEN(const std::string& fenString);

	// making and unmaking mvoes
	bool makeMove(MoveData* moveData);
	bool unmakeMove(MoveData* moveData, bool positionUpdated = true);
	void promotePiece(MoveData* md, MoveData::EncodingBits promoteTo);

	// fetching moves for a side from MoveGenerator
	void calculateSideMoves(Colour side);
	void calculateSideMovesCapturesOnly(Colour side);
	
	Byte computeKingSquare(Bitboard kingBB);
	bool squareAttacked(Byte square, Colour attackingSide);

	ZobristKey* getZobristKeyHistory()	   { return mZobristKeyHistory; }
	short getCurrentPly()				   { return mPly;				}
	short getFiftyMoveCounter()			   { return mFiftyMoveCounter;  }
	std::vector<MoveData>& getMovesRef(Colour side);
};
