#pragma once

#include <string>
#include <vector>

#include "Bitboard.h"
#include "ChessPosition.h"
#include "MoveData.h"
#include "MoveGeneration.h"
#include "ZobristKey.h"

// this class handles all of the piece movement, position updating, as well as some additional utility functions for Athena or the UCI handler
class Board
{    
private:
	// stores the zobrist key of the current position
	ZobristKey::zkey mCurrentZobristKey;

	// each index contains the zobrist key of the board's position at that ply in the game's history
	ZobristKey::zkey mZobristKeyHistory[1000];

	// stores the current ply (i.e., how many half-moves have occured so far)
	short mPly;

	void initializeAuxillaryBitboards();

	void setCastleMoveData(MoveData* castleMoveData, MoveData* kingMD, MoveData* rookMD);
	bool makeCastleMove(MoveData* moveData);
	void setEnPassantSquares(MoveData* moveData);
	void updateBitboardWithMove(MoveData* moveData);
	void undoPromotion(MoveData* moveData);

	void insertMoveIntoHistory(short ply);
	void deleteMoveFromHistory(short ply);

	void setFENPiecePlacement(char pieceType, Byte square);
	std::string getSquareStringCoordinate(Byte square);
	Byte getSquareNumberCoordinate(const std::string& stringCoordinate);

public:
	Board() {}

	// contains all the information about the current position of the board
	ChessPosition currentPosition;

	void init();

	std::string getMoveLANString(MoveData* moveData);
	bool makeMoveLAN(const std::string& lanString);
	void setPositionFEN(const std::string& fenString);

	bool makeMove(MoveData* moveData);
	bool unmakeMove(MoveData* moveData, bool positionUpdated = true);
	void promotePiece(MoveData* md, MoveData::EncodingBits promoteTo);
	
	Byte computeKingSquare(Bitboard kingBB);
	bool squareAttacked(Byte square, Colour attackingSide);

	void getLeastValuableAttacker(Byte square, Colour attackingSide, int* pieceValue, Bitboard** pieceBB, Bitboard* attackingPiecesBB);

	ZobristKey::zkey* getZobristKeyHistory()		{ return mZobristKeyHistory;							 }
	short getCurrentPly()							{ return mPly;											 }
	short getFiftyMoveCounter()					    { return currentPosition.fiftyMoveCounter;				 }
};