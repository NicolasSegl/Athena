#include <cmath>
#include <iostream>
#include <random>

#include "Board.h"
#include "MoveGeneration.h"
#include "Outcomes.h"
#include "utils.h"

namespace FenDataFields
{
	enum FEN_FIELDS
	{
		PIECE_PLACEMENT,
		SIDE_TO_PLAY,
		CASTLE_PRIVILEGES,
		EN_PASSANT_SQUARE,
		FIFTY_MOVE_COUNTER,
		NUM_FULL_MOVES,
	};
}

namespace ASCII
{
	const int NUMBER_ONE_CODE = 49;
	const int LETTER_A_CODE = 97;
}

// sets appropriate bits on white/black pieces bitboards and occupied/empty bitboards
void Board::setAuxillaryBitboards()
{
	currentPosition.whitePiecesBB  = currentPosition.whitePawnsBB | currentPosition.whiteRooksBB | currentPosition.whiteKnightsBB | 
									 currentPosition.whiteBishopsBB | currentPosition.whiteQueensBB | currentPosition.whiteKingBB;

	currentPosition.blackPiecesBB  = currentPosition.blackPawnsBB | currentPosition.blackRooksBB | currentPosition.blackKnightsBB | 
									 currentPosition.blackBishopsBB | currentPosition.blackQueensBB | currentPosition.blackKingBB;

	currentPosition.occupiedBB = currentPosition.whitePiecesBB | currentPosition.blackPiecesBB;
	currentPosition.emptyBB	   = ~currentPosition.occupiedBB;
}

// sets a specific bit on the appropriate piece bitboard based on the character and square in the FEN string
void Board::setFENPiecePlacement(char pieceType, Byte square)
{
	switch (pieceType)
	{
		case 'P': currentPosition.whitePawnsBB	 |= BB::boardSquares[square]; break;
		case 'p': currentPosition.blackPawnsBB	 |= BB::boardSquares[square]; break;
		case 'N': currentPosition.whiteKnightsBB |= BB::boardSquares[square]; break;
		case 'n': currentPosition.blackKnightsBB |= BB::boardSquares[square]; break;
		case 'B': currentPosition.whiteBishopsBB |= BB::boardSquares[square]; break;
		case 'b': currentPosition.blackBishopsBB |= BB::boardSquares[square]; break;
		case 'R': currentPosition.whiteRooksBB   |= BB::boardSquares[square]; break;
		case 'r': currentPosition.blackRooksBB   |= BB::boardSquares[square]; break;
		case 'Q': currentPosition.whiteQueensBB  |= BB::boardSquares[square]; break;
		case 'q': currentPosition.blackQueensBB  |= BB::boardSquares[square]; break;
		case 'K': currentPosition.whiteKingBB	 |= BB::boardSquares[square]; break;
		case 'k': currentPosition.blackKingBB	 |= BB::boardSquares[square]; break;
	}
}

// using ascii text manipulation to find the little endian file mapping coordinate from a letter/number chess coordinate (i.e. a4, h3)
Byte Board::getSquareNumberCoordinate(const std::string& stringCoordinate)
{
	// calculates the square by multiplying the number (row/rank) by 8, then adding the number (column/file)
	return (stringCoordinate[1] - ASCII::NUMBER_ONE_CODE) * 8 + (stringCoordinate[0] - ASCII::LETTER_A_CODE);
}

// converts little endian file mapping coordinate to letter/number chess coordinate (also using some ascii text manipulation)
std::string Board::getSquareStringCoordinate(Byte square)
{
	std::string moveString;
	moveString.resize(2);

	moveString[0] = (char)(ASCII::LETTER_A_CODE   + square % 8); // gets the letter
 	moveString[1] = (char)(ASCII::NUMBER_ONE_CODE + square / 8); // gets the number

	return moveString;
}

// interprets all the data from an FEN string. this includes:
// setting appropriate bits in associated bitboards (i.e. white pawns in the white pawns bitboard, setting occupied bitboard, etc)
// taking side to move, en passant square, half-move counter, and castle privileges data into account  
void Board::setPositionFEN(const std::string& fenString)
{
	currentPosition.reset();

	// loop through all the chatacters of the FEN string and set the piece placement data into the engine's abstractions
	int column = 0;
	int row    = 7;
	for (int character = 0; character < fenString.size(); character++)
	{
		// if we get to the data past the placement data there will be a space
		if (fenString[character] == ' ')
			break;
		else if (fenString[character] == '/')
		{
			row--;
			column = 0;
		}
		else if (fenString[character] > 57) // an actual letter depicting a piece (not a number)
		{
			setFENPiecePlacement(fenString[character], row * 8 + column);
			column++;
		}
		else // a number depicting how many empty squares until a piece
			column += fenString[character] - 48;
	}

	// fill dataVec with the remaining data (en passant square, castle privileges, half-move counter, side to move) and parse into engine
	std::vector<std::string> dataVec;
	splitString(fenString, dataVec, ' ');

	// set side to move and adjust current ply (using the total number of full moves at the end of the FEN data)
	if (dataVec[FenDataFields::SIDE_TO_PLAY][0] == 'w')
	{
		mPly = -1; // so that when we add 2 times the number of full moves, we get the proper ply if the side to move is white
		currentPosition.sideToMove = SIDE_WHITE;
	}
	else
		currentPosition.sideToMove = SIDE_BLACK;

	// set castle privileges 
	for (int character = 0; character < dataVec[FenDataFields::CASTLE_PRIVILEGES].size(); character++)
	{
		if		(dataVec[FenDataFields::CASTLE_PRIVILEGES][character] == 'k') currentPosition.castlePrivileges |= (Byte)CastlingPrivilege::BLACK_SHORT_CASTLE;
		else if (dataVec[FenDataFields::CASTLE_PRIVILEGES][character] == 'q') currentPosition.castlePrivileges |= (Byte)CastlingPrivilege::BLACK_LONG_CASTLE;
		else if (dataVec[FenDataFields::CASTLE_PRIVILEGES][character] == 'K') currentPosition.castlePrivileges |= (Byte)CastlingPrivilege::WHITE_SHORT_CASTLE;
		else if (dataVec[FenDataFields::CASTLE_PRIVILEGES][character] == 'Q') currentPosition.castlePrivileges |= (Byte)CastlingPrivilege::WHITE_LONG_CASTLE;
	}

	// set en passant square
	if (dataVec[FenDataFields::EN_PASSANT_SQUARE] != "-")
		currentPosition.enPassantSquare = getSquareNumberCoordinate(dataVec[FenDataFields::EN_PASSANT_SQUARE]);

	// set the number of moves
	currentPosition.fiftyMoveCounter = std::stoi(dataVec[FenDataFields::FIFTY_MOVE_COUNTER]);

	// set the current ply
	mPly += 2 * std::stoi(dataVec[FenDataFields::NUM_FULL_MOVES]);

	insertMoveIntoHistory(mPly);

	setAuxillaryBitboards();
}

// make a move formatted long algebraic notation (for uci purposes)
bool Board::makeMoveLAN(const std::string& lanString)
{
	// disect the LAN string into a string for the origin square and the target square
	std::string from(lanString.begin(), lanString.begin() + 2);
	std::string to(lanString.begin() + 2, lanString.begin() + 4);

	// convert the above strings into number coordinates
	Byte moveOriginSquare = getSquareNumberCoordinate(from);
	Byte moveTargetSquare = getSquareNumberCoordinate(to);

	std::vector<MoveData> moveVec;
	MoveGeneration::calculatePieceMoves(this, currentPosition.sideToMove, moveOriginSquare, moveVec, false);
	if (from == "e1" || from == "e8")
		MoveGeneration::calculateCastleMoves(this, currentPosition.sideToMove, moveVec);

	for (int i = 0; i < moveVec.size(); i++)
		// compare the origin/target squares of the move provided with the origin/target squares of the possible moves at that square (plus potential castle moves)
		// if there's a match, then make the move (using the move data calculated above)
		if (moveVec[i].originSquare == moveOriginSquare && moveVec[i].targetSquare == moveTargetSquare)
		{
			makeMove(&moveVec[i]);
			char lastCharacter = lanString.back();
			// if the last character in the lan move string is a character, then it means a pawn has promoted
			if (lastCharacter > ASCII::LETTER_A_CODE)
			{
				if		(lastCharacter == 'q') promotePiece(&moveVec[i], MoveData::EncodingBits::QUEEN_PROMO);
				else if (lastCharacter == 'r') promotePiece(&moveVec[i], MoveData::EncodingBits::ROOK_PROMO);
				else if (lastCharacter == 'n') promotePiece(&moveVec[i], MoveData::EncodingBits::KNIGHT_PROMO);
				else if (lastCharacter == 'b') promotePiece(&moveVec[i], MoveData::EncodingBits::BISHOP_PROMO);
			}

			return true;
		}

	return false;
}

// convert the engine's move dat astructure into a LAN string (excluding the type of piece at the start of the string, as uci 
// does not require it, while including the promotion of the pawn should it be needed)
std::string Board::getMoveLANString(MoveData* moveData)
{
	std::string lanString = getSquareStringCoordinate(moveData->originSquare) + getSquareStringCoordinate(moveData->targetSquare);

	if		(moveData->moveType == MoveData::EncodingBits::QUEEN_PROMO)  lanString += "q";
	else if (moveData->moveType == MoveData::EncodingBits::ROOK_PROMO)   lanString += "r";
	else if (moveData->moveType == MoveData::EncodingBits::KNIGHT_PROMO) lanString += "n";
	else if (moveData->moveType == MoveData::EncodingBits::BISHOP_PROMO) lanString += "b";

	return lanString;
}

// initialize the bitboards, move generator tables, etc.
// this function is a bit old and requires updating as some of it is redundant
void Board::init()
{
	BB::initialize();
	MoveGeneration::init();

	mZobristKeyGenerator.initHashKeys();
	mCurrentZobristKey = mZobristKeyGenerator.generateKey(&currentPosition);
}

// set the basic move data (origin/target square, colour bitboard, and piece bitboard) of the two moves made during a castle move
void Board::setCastleMoveData(MoveData* castleMoveData, MoveData* kingMD, MoveData* rookMD)
{
	kingMD->originSquare = castleMoveData->originSquare;
	kingMD->targetSquare = castleMoveData->targetSquare;
	rookMD->setMoveType(MoveData::EncodingBits::CASTLE_HALF_MOVE);
	kingMD->setMoveType(MoveData::EncodingBits::CASTLE_HALF_MOVE);

    if (castleMoveData->side == SIDE_WHITE)
    {
        kingMD->pieceBB  = &currentPosition.whiteKingBB;
        kingMD->colourBB = &currentPosition.whitePiecesBB;

        rookMD->pieceBB  = &currentPosition.whiteRooksBB;
        rookMD->colourBB = &currentPosition.whitePiecesBB;
    }
    else // the move is from the black side
    {
        kingMD->pieceBB  = &currentPosition.blackKingBB;
        kingMD->colourBB = &currentPosition.blackPiecesBB;

        rookMD->pieceBB  = &currentPosition.blackRooksBB;
        rookMD->colourBB = &currentPosition.blackPiecesBB;
    }
}

// if the castle move is legal, this function will make 2 moves that will mimic a castle move (by moving the king and the rook in one move)
bool Board::makeCastleMove(MoveData* md)
{
    static MoveData kingMove;
    static MoveData rookMove;
    setCastleMoveData(md, &kingMove, &rookMove); // sets the origin/target square, colour bitboard, and piece bitboard of the two "half" moves

    if (md->moveType == MoveData::EncodingBits::SHORT_CASTLE)
    {
        // prevent castling if squares are under attack (preventing a caslte)
        for (int square = 0; square <= 2; square++)
            if (squareAttacked(md->originSquare + square, !md->side))
                return false;
        
        rookMove.originSquare = md->originSquare + 3;
        rookMove.targetSquare = md->targetSquare - 1;
    }
    else if (md->moveType == MoveData::EncodingBits::LONG_CASTLE)
    {
        // prevent castling if squares are under attack (preventing a castle)
        for (int square = 0; square <= 2; square++)
            if (squareAttacked(md->originSquare - square, !md->side)) // if attackTable[squrea] & BB::boardsquares[square] ?
                return false;
        
        rookMove.originSquare = md->originSquare - 4;
        rookMove.targetSquare = md->targetSquare + 1;
    }
    else
        return false;

	// if this function is being called by unmake move (can tell by checking if the rook is not on the corner squares),
	// then call the appropriate make/unmake move function
	if (BB::boardSquares[rookMove.targetSquare] & *rookMove.pieceBB)
	{
		unmakeMove(&kingMove);
		unmakeMove(&rookMove);
		return true;
	}
	else
	{
		makeMove(&kingMove);
		makeMove(&rookMove);
	}

    return true;
}

// after a move, use bitwise operators to set or unset the appropriate bits in the bitboards affected by the move
void Board::updateBitboardWithMove(MoveData* moveData)
{
	Bitboard originBB = BB::boardSquares[moveData->originSquare];
	Bitboard targetBB = BB::boardSquares[moveData->targetSquare];
	Bitboard originTargetBB = originBB ^ targetBB; // 1 on from and to, 0 on everything else

	// unsets the origin square and sets the target square
	*moveData->pieceBB  ^= originTargetBB;
	*moveData->colourBB ^= originTargetBB;

	if (moveData->capturedPieceBB) // if a piece was captured
	{
		if (moveData->moveType != MoveData::EncodingBits::EN_PASSANT_CAPTURE)
		{
			// unset the bit that was captured
			*moveData->capturedPieceBB  ^= targetBB;
			*moveData->capturedColourBB ^= targetBB; 

			// the origin square is now empty. we do not XOR the target square, however, as it was previously occupied so to XOR it would unset the bit
			currentPosition.occupiedBB  ^= originBB;
			currentPosition.emptyBB	    ^= originBB;
		}
		else // the move was an en passant capture
		{
			// as the target square is not actually the location of the piece for an en passant capture, we need to find the square of the victim
			Bitboard capturedPieceBB = moveData->side == SIDE_WHITE ? (targetBB >> 8) : (targetBB << 8);

			// unset the bit that was captured
			*moveData->capturedPieceBB  ^= capturedPieceBB;
			*moveData->capturedColourBB ^= capturedPieceBB;

			// unset the bit for the origin square, the square of the captured piece, and fill in the en passant square
			currentPosition.occupiedBB ^= originBB | capturedPieceBB | targetBB; 
			currentPosition.emptyBB	   ^= originBB | capturedPieceBB | targetBB;
		}
	}
	else // not a capture move
	{
		// unset the origin square and set the target square
		currentPosition.occupiedBB ^= originTargetBB;
		currentPosition.emptyBB	   ^= originTargetBB;
	}
}

// loop through the board until the square of the king is found
Byte Board::computeKingSquare(Bitboard kingBB)
{
	return BB::getLSB(kingBB);
}

/* 
   returns true if the specified square is attacked, false otherwise
   it achieves this by making bitboards containing bits set for all pieces capable of making select moves (diagonal, straight, etc) for a certain side
   it then compares these bitboards to the possible moves such pieces could make at the square's location
 
   for example, bishops and queens can attack on diagonals, so we make a bitboard containing all of the bishops and queens.
   afterwards, we AND this bitboard with the possible diagonal moves from the square being attacked. if the value is > 0, 
   then either a queen or a bishop has a diagonal attack that is hitting the square (so return true)
*/
bool Board::squareAttacked(Byte square, Colour attackingSide)
{
	Bitboard opPawnsBB = attackingSide == SIDE_WHITE ? currentPosition.whitePawnsBB : currentPosition.blackPawnsBB;

	// this gets all of the diagonal attacks of the pawns depending on the side attacking. by clearing the A and H files,
	// we are preventing the "attack" from overflowing into the file on the opposite end of the board
	if (attackingSide == SIDE_WHITE)
	{
		if ((BB::boardSquares[square] & ((opPawnsBB & BB::fileClear[BB::FILE_A]) << 7)) || (BB::boardSquares[square] & ((opPawnsBB & BB::fileClear[BB::FILE_H]) << 9))) 
			return true;
	}
	else
	{
		if ((BB::boardSquares[square] & ((opPawnsBB & BB::fileClear[BB::FILE_A]) >> 9)) || (BB::boardSquares[square] & ((opPawnsBB & BB::fileClear[BB::FILE_H]) >> 7)))
			return true;
	}
    
    Bitboard opKnightsBB = attackingSide == SIDE_WHITE ? currentPosition.whiteKnightsBB : currentPosition.blackKnightsBB;
    if (MoveGeneration::knightLookupTable[square] & opKnightsBB)                  return true;
    
    Bitboard opKingsBB = attackingSide    == SIDE_WHITE ? currentPosition.whiteKingBB : currentPosition.blackKingBB;
    if (MoveGeneration::kingLookupTable[square] & opKingsBB)                      return true;
    
    Bitboard opBishopsQueensBB = attackingSide == SIDE_WHITE ? currentPosition.whiteBishopsBB | currentPosition.whiteQueensBB  
															 : currentPosition.blackBishopsBB | currentPosition.blackQueensBB;
    Bitboard friendlyPieces = attackingSide == SIDE_WHITE ? currentPosition.blackPiecesBB : currentPosition.whitePiecesBB;
    if (MoveGeneration::computePseudoBishopMoves(square, currentPosition.occupiedBB, friendlyPieces) & opBishopsQueensBB) return true;
    
    Bitboard opRooksQueens = attackingSide == SIDE_WHITE ? currentPosition.whiteRooksBB | currentPosition.whiteQueensBB  
														 : currentPosition.blackRooksBB | currentPosition.blackQueensBB;
    if (MoveGeneration::computePseudoRookMoves(square, currentPosition.occupiedBB, friendlyPieces) & opRooksQueens) return true;
    
    return false;
}

// using the same methods as Board::squareAttacked(), this function returns the value of the least valuable piece attacking the square as well as its piece bitboard
// it is ordered differently, however, as to make sure the least valuable attackers are considered first
void Board::getLeastValuableAttacker(Byte square, Colour attackingSide, int* pieceValue, Bitboard** pieceBB, Bitboard* pieceAttacksBB)
{
	Bitboard opPawnsBB = attackingSide == SIDE_WHITE ? currentPosition.whitePawnsBB : currentPosition.blackPawnsBB;

	// this gets all of the diagonal attacks of the pawns depending on the side attacking. by clearing the A and H files,
	// we are preventing the "attack" from overflowing into the file on the opposite end of the board
	if (attackingSide == SIDE_WHITE)
	{
		*pieceAttacksBB = (((opPawnsBB & BB::fileClear[BB::FILE_A]) << 7) | ((opPawnsBB & BB::fileClear[BB::FILE_H]) << 9)) & BB::boardSquares[square];
		if (*pieceAttacksBB)
		{
			*pieceAttacksBB = (((opPawnsBB & BB::fileClear[BB::FILE_A]) << 7) >> 7) | (((opPawnsBB & BB::fileClear[BB::FILE_H]) << 9) >> 9);
			*pieceValue = 100;
			*pieceBB = &currentPosition.whitePawnsBB;
			return;
		}
	}
	else
	{
		*pieceAttacksBB = (((opPawnsBB & BB::fileClear[BB::FILE_A]) >> 9) | ((opPawnsBB & BB::fileClear[BB::FILE_H]) >> 7)) & BB::boardSquares[square];
		if (*pieceAttacksBB)
		{
			*pieceAttacksBB = (((opPawnsBB & BB::fileClear[BB::FILE_A]) >> 9) << 9) | (((opPawnsBB & BB::fileClear[BB::FILE_H]) >> 7) << 7);
			*pieceValue = 100;
			*pieceBB = &currentPosition.blackPawnsBB;
			return;
		}
	}

	Bitboard* opKnightsBB = attackingSide == SIDE_WHITE ? &currentPosition.whiteKnightsBB : &currentPosition.blackKnightsBB;
	*pieceAttacksBB = MoveGeneration::knightLookupTable[square] & *opKnightsBB;
	if (*pieceAttacksBB)
	{
		*pieceValue = 320;
		*pieceBB = opKnightsBB;
		return;
	}

	Bitboard friendlyPieces = attackingSide == SIDE_WHITE ? currentPosition.blackPiecesBB : currentPosition.whitePiecesBB;

	Bitboard* opBishopsBB = attackingSide == SIDE_WHITE ? &currentPosition.whiteBishopsBB : &currentPosition.blackBishopsBB;
	Bitboard bishopMovesBB = MoveGeneration::computePseudoBishopMoves(square, currentPosition.occupiedBB, friendlyPieces);
	*pieceAttacksBB = bishopMovesBB & *opBishopsBB;
	if (*pieceAttacksBB)
	{
		*pieceValue = 330;
		*pieceBB = opBishopsBB;
		return;
	}

	Bitboard* opRooks = attackingSide == SIDE_WHITE ? &currentPosition.whiteRooksBB : &currentPosition.blackRooksBB;
	Bitboard rookMovesBB = MoveGeneration::computePseudoRookMoves(square, currentPosition.occupiedBB, friendlyPieces);
	*pieceAttacksBB = rookMovesBB & *opRooks;
	if (*pieceAttacksBB)
	{
		*pieceValue = 500;
		*pieceBB = opRooks;
		return;
	}

	Bitboard* opQueens = attackingSide == SIDE_WHITE ? &currentPosition.whiteQueensBB : &currentPosition.blackQueensBB;
	*pieceAttacksBB = (rookMovesBB | bishopMovesBB) & *opQueens;
	if (*pieceAttacksBB)
	{
		*pieceValue = 900;
		*pieceBB = opQueens;
		return;
	}

	Bitboard* opKingsBB = attackingSide == SIDE_WHITE ? &currentPosition.whiteKingBB : &currentPosition.blackKingBB;
	*pieceAttacksBB = MoveGeneration::kingLookupTable[square] & *opKingsBB;
	if (*pieceAttacksBB)
	{
		*pieceValue = 20000;
		*pieceBB = opKingsBB;
		return;
	}
}

// if the move made generated an en passant square, set the current en passant square for the current position
void Board::setEnPassantSquares(MoveData* moveData)
{
	currentPosition.enPassantSquare = 0;
	if (moveData->pieceBB == &currentPosition.whitePawnsBB && moveData->targetSquare - moveData->originSquare == 16) // if move is en passant
		currentPosition.enPassantSquare = moveData->targetSquare - 8;
	else if (moveData->pieceBB == &currentPosition.blackPawnsBB && moveData->originSquare - moveData->targetSquare == 16)
		currentPosition.enPassantSquare = moveData->targetSquare + 8;
}

void Board::insertMoveIntoHistory(short ply)
{
	mZobristKeyHistory[ply] = mCurrentZobristKey;
}

void Board::deleteMoveFromHistory(short ply)
{
	mZobristKeyHistory[ply] = 0;
}

/* 
    using the information in the MoveData struct, perform the following:
		update the bitboards affected in the move
		check legality of the move (i.e. if it would result in a check)
		update castle privileges
		set any en passant squares, update the fifty move counter, and change the side to move
		insert the move into the move history (unless it was one of the two castle moves making up the whole move)
*/
bool Board::makeMove(MoveData* moveData)
{
	if (moveData->moveType == MoveData::EncodingBits::SHORT_CASTLE || moveData->moveType == MoveData::EncodingBits::LONG_CASTLE)
	{
		if (!makeCastleMove(moveData))
			return false;
	}
	else // a non-castle move
	{
		updateBitboardWithMove(moveData);

		Byte kingSquare = computeKingSquare(moveData->side == SIDE_WHITE ? currentPosition.whiteKingBB : currentPosition.blackKingBB);

		if (squareAttacked(kingSquare, !moveData->side) || kingSquare == 255) // kingSquare == 255 means that there is no king on the board
		{
			// passing in false so that we do not update the zobrist key/history, as we never actually added it here
			// this means that otherwise it would be removing the previous zobrist key, eventually giving negative 
			// ply numbers and undefined behaviour
			unmakeMove(moveData, false);
			return false;
		}
	}

	setEnPassantSquares(moveData);

	currentPosition.castlePrivileges &= ~moveData->castlePrivilegesRevoked;
	if (moveData->moveType != MoveData::EncodingBits::CASTLE_HALF_MOVE)
	{
		moveData->fiftyMoveCounter = currentPosition.fiftyMoveCounter;

		// reset the fifty move counter if there was a capture or a pawn advance, otherwise increment it
		if (moveData->capturedPieceBB || moveData->pieceBB == &currentPosition.whitePawnsBB || moveData->pieceBB == &currentPosition.blackPawnsBB)
			currentPosition.fiftyMoveCounter = 0;
		else
			currentPosition.fiftyMoveCounter++;

		currentPosition.sideToMove = !currentPosition.sideToMove;
		mCurrentZobristKey = mZobristKeyGenerator.updateKey(mCurrentZobristKey, &currentPosition, moveData);
		insertMoveIntoHistory(++mPly);
	}
	else
		mCurrentZobristKey = mZobristKeyGenerator.updateKey(mCurrentZobristKey, &currentPosition, moveData);

	return true;
}

/*
	using the information in the MoveData struct, this functions takes a move back. It does so by:
		undoing any pawn promotions
		updating the bitboards that were affected in the move
		reseting en passant squares, castle privileges, side to move, and fifty move counter by using the values stored in the move data
		deleting the move from the move history
*/
bool Board::unmakeMove(MoveData* moveData, bool positionUpdated)
{
	// undoing moves is no longer returning the taken pieces back ? 
	if (moveData->moveType == MoveData::EncodingBits::SHORT_CASTLE || moveData->moveType == MoveData::EncodingBits::LONG_CASTLE)
		makeCastleMove(moveData);
	else
	{
		undoPromotion(moveData);
		updateBitboardWithMove(moveData);
	}

	if (moveData->moveType != MoveData::EncodingBits::CASTLE_HALF_MOVE && positionUpdated)
	{
		mCurrentZobristKey = mZobristKeyHistory[mPly - 1];

		currentPosition.enPassantSquare = moveData->enPassantSquare;
		currentPosition.castlePrivileges ^= moveData->castlePrivilegesRevoked;
		currentPosition.fiftyMoveCounter = moveData->fiftyMoveCounter;
		currentPosition.sideToMove = !currentPosition.sideToMove;
		deleteMoveFromHistory(mPly--);
	}

	return true;
}

// replaces the promoted pawn back to a pawn and removes the piece the pawn promoted to from its respective bitboard
void Board::undoPromotion(MoveData* moveData)
{
	if (moveData->moveType == MoveData::EncodingBits::BISHOP_PROMO || moveData->moveType == MoveData::EncodingBits::ROOK_PROMO ||
		moveData->moveType == MoveData::EncodingBits::KNIGHT_PROMO || moveData->moveType == MoveData::EncodingBits::QUEEN_PROMO)
	{
		if (moveData->side == SIDE_WHITE) currentPosition.whitePawnsBB ^= BB::boardSquares[moveData->targetSquare];
		else							  currentPosition.blackPawnsBB ^= BB::boardSquares[moveData->targetSquare];
	}
	else
		return;

	switch (moveData->moveType)
	{
		case MoveData::EncodingBits::QUEEN_PROMO:
		{
			if (moveData->side == SIDE_WHITE) currentPosition.whiteQueensBB ^= BB::boardSquares[moveData->targetSquare];
			else							  currentPosition.blackQueensBB ^= BB::boardSquares[moveData->targetSquare];
			break;
		}
		case MoveData::EncodingBits::BISHOP_PROMO:
		{
			if (moveData->side == SIDE_WHITE) currentPosition.whiteBishopsBB ^= BB::boardSquares[moveData->targetSquare];
			else							  currentPosition.blackBishopsBB ^= BB::boardSquares[moveData->targetSquare];
			break;
		}
		case MoveData::EncodingBits::ROOK_PROMO:
		{
			if (moveData->side == SIDE_WHITE) currentPosition.whiteRooksBB ^= BB::boardSquares[moveData->targetSquare];
			else							  currentPosition.blackRooksBB ^= BB::boardSquares[moveData->targetSquare];
			break;
		}
		case MoveData::EncodingBits::KNIGHT_PROMO:
		{
			if (moveData->side == SIDE_WHITE) currentPosition.whiteKnightsBB ^= BB::boardSquares[moveData->targetSquare];
			else							  currentPosition.blackKnightsBB ^= BB::boardSquares[moveData->targetSquare];
			break;
		}
		default:
			break;
	}
}

// removes pawn from its bitboard via XORING then adds the piece the pawn promoted to 
// to its respective bitboard via ORING the pawn's target square (the back rank)
void Board::promotePiece(MoveData* md, MoveData::EncodingBits promoteTo)
{
	md->setMoveType(promoteTo);

	// make this a switch statement?
	if (md->side == SIDE_WHITE)
	{
		currentPosition.whitePawnsBB ^= BB::boardSquares[md->targetSquare];

		if		(promoteTo == MoveData::EncodingBits::QUEEN_PROMO)	currentPosition.whiteQueensBB  |= BB::boardSquares[md->targetSquare];
		else if (promoteTo == MoveData::EncodingBits::ROOK_PROMO)	currentPosition.whiteRooksBB   |= BB::boardSquares[md->targetSquare];
		else if (promoteTo == MoveData::EncodingBits::BISHOP_PROMO) currentPosition.whiteBishopsBB |= BB::boardSquares[md->targetSquare];
		else if (promoteTo == MoveData::EncodingBits::KNIGHT_PROMO) currentPosition.whiteKnightsBB |= BB::boardSquares[md->targetSquare];
	}
	else // side == SIDE_BLACK
	{
		currentPosition.blackPawnsBB ^= BB::boardSquares[md->targetSquare];

		if		(promoteTo == MoveData::EncodingBits::QUEEN_PROMO)	currentPosition.blackQueensBB  |= BB::boardSquares[md->targetSquare];
		else if (promoteTo == MoveData::EncodingBits::ROOK_PROMO)	currentPosition.blackRooksBB   |= BB::boardSquares[md->targetSquare];
		else if (promoteTo == MoveData::EncodingBits::BISHOP_PROMO) currentPosition.blackBishopsBB |= BB::boardSquares[md->targetSquare];
		else if (promoteTo == MoveData::EncodingBits::KNIGHT_PROMO) currentPosition.blackKnightsBB |= BB::boardSquares[md->targetSquare];
	}
}