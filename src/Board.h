#pragma once

#include <string>
#include <vector>

#include "Bitboard.h"
#include "ChessPosition.h"
#include "MoveData.h"
#include "MoveGeneration.h"
#include "ZobristKeyGenerator.h"

class Board
{    
private:
	std::vector<MoveData> mWhiteMoves;
	std::vector<MoveData> mBlackMoves;

	uint64_t mCurrentZobristKey;
	ZobristKeyGenerator mZobristKeyGenerator;
	ZobristKey mZobristKeyHistory[1000];

	short mPly;

	void setAuxillaryBitboards();

	// functions for making/unmaking moves
	void setCastleMoveData(MoveData* castleMoveData, MoveData* kingMD, MoveData* rookMD);
	bool makeCastleMove(MoveData* moveData);
	void setEnPassantSquares(MoveData* moveData);
	void updateBitboardWithMove(MoveData* moveData);
	void undoPromotion(MoveData* moveData);

	// zobrist keys
	void insertMoveIntoHistory(short ply);
	void deleteMoveFromHistory(short ply);

	// fen strings
	void setFENPiecePlacement(char pieceType, Byte square);
	std::string getSquareStringCoordinate(Byte square);
	Byte getSquareNumberCoordinate(const std::string& stringCoordinate);

public:
	Board() {}

	ChessPosition currentPosition;

	void init();

	// functions interpretting different chess notations (mainly for uci/opening book usage)
	std::string getMoveLANString(MoveData* moveData);
	bool makeMoveLAN(const std::string& lanString);
	void setPositionFEN(const std::string& fenString);

	// making and unmaking mvoes
	bool makeMove(MoveData* moveData);
	bool unmakeMove(MoveData* moveData, bool positionUpdated = true);
	void promotePiece(MoveData* md, MoveData::EncodingBits promoteTo);
	
	Byte computeKingSquare(Bitboard kingBB);
	bool squareAttacked(Byte square, Colour attackingSide);

	// for static search evaluation
	void getLeastValuableAttacker(Byte square, Colour attackingSide, int* pieceValue, Bitboard** pieceBB, Bitboard* pieceAttacksBB);

	ZobristKey* getZobristKeyHistory()				{ return mZobristKeyHistory;							 }
	short getCurrentPly()							{ return mPly;											 }
	short getFiftyMoveCounter()					    { return currentPosition.fiftyMoveCounter;				 }
	std::vector<MoveData>& getMovesRef(Colour side) { return side == SIDE_WHITE ? mWhiteMoves : mBlackMoves; }
};