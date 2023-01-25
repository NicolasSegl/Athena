#include <cmath>
#include <iostream>
#include <random>

#include "Board.h"
#include "Constants.h"
#include "Eval.h"
#include "MoveGeneration.h"
#include "Outcomes.h"
#include "utils.h"

// this namespace and enum represent the format of the FEN string passed in for setting the board position
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

// sets appropriate bits on white/black pieces bitboards and occupied/empty bitboards
void Board::initializeAuxillaryBitboards()
{
	currentPosition.whitePiecesBB  = currentPosition.whitePawnsBB | currentPosition.whiteRooksBB | currentPosition.whiteKnightsBB | 
									 currentPosition.whiteBishopsBB | currentPosition.whiteQueensBB | currentPosition.whiteKingBB;

	currentPosition.blackPiecesBB  = currentPosition.blackPawnsBB | currentPosition.blackRooksBB | currentPosition.blackKnightsBB | 
									 currentPosition.blackBishopsBB | currentPosition.blackQueensBB | currentPosition.blackKingBB;

	currentPosition.occupiedBB = currentPosition.whitePiecesBB | currentPosition.blackPiecesBB;
	currentPosition.emptyBB	   = ~currentPosition.occupiedBB;
}

// sets a specific bit on the appropriate piece bitboard based on the character and square passed in by the FEN string
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

// uses ascii text manipulation to find the little endian file mapping coordinate from a letter/number chess coordinate (i.e. a4, h3)
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
// taking side to move, en passant square, half-move counter, and castle privileges into account  
void Board::setPositionFEN(const std::string& fenString)
{
	currentPosition.reset();

	// loop through all the chatacters of the FEN string and set the positions of the pieces into the engine's abstractions
	int column = 0;
	int row    = 7;
	for (int character = 0; character < fenString.size(); character++)
	{
		// if we get to the data past the placement data there will be a space
		if (fenString[character] == ' ')
			break;
		// a slash indicates a new row (so go down a row and start again from the leftmost column)
		else if (fenString[character] == '/')
		{
			row--;
			column = 0;
		}
		// a character that is a letter indicates that there exists a piece in that position 
		else if (fenString[character] > ASCII::NUMBER_NINE_CODE)
		{
			setFENPiecePlacement(fenString[character], row * 8 + column);
			column++;
		}
		// a number indicates how many empty squares there are until yet another piece on that rank (or just until the end of the rank, if there are no other pieces)
		else
			column += fenString[character] - 48;
	}

	// fill a vector with the remaining data (en passant square, castle privileges, half-move counter, side to move) and parse into engine
	std::vector<std::string> dataVec;
	splitString(fenString, dataVec, ' ');

	// set side to move and adjust current ply (using the total number of full moves at the end of the FEN data)
	if (dataVec[FenDataFields::SIDE_TO_PLAY][0] == 'w')
	{
		mPly = -2; // so that when we add 2 times the number of full moves, we get the proper ply if the side to move is white
		currentPosition.sideToMove = SIDE_WHITE;
	}
	else
	{
		mPly = -1; // same as the most recent comment, only for if the side to move is black
		currentPosition.sideToMove = SIDE_BLACK;
	}

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

	// set the bits on all other additional bitboards for the current position (like the occupied bitboard or the white pieces bitboard)
	initializeAuxillaryBitboards();

	// generate a new zobrist key based off of the position and insert it into the position history at the current ply
	mCurrentZobristKey = ZobristKey::generate(&currentPosition);
	insertMoveIntoHistory(mPly);
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

	// populate a vector with all the possible moves
	std::vector<MoveData> moveVec;
	MoveGeneration::calculatePieceMoves(this, currentPosition.sideToMove, moveOriginSquare, moveVec, false);
	if (from == "e1" || from == "e8")
		MoveGeneration::calculateCastleMoves(this, currentPosition.sideToMove, moveVec);

	// iterate over all the moves and compare the origin/target squares of the move provided with the origin/target squares of the
	// possible moves at that square (plus potential castle moves) if there's a match, then make the move
	for (int i = 0; i < moveVec.size(); i++)
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

// convert the engine's move data structure into a LAN string (excluding the type of piece at the start of the string, as uci 
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

// initialize the bitboards and move generator tables
void Board::init()
{
	BB::initialize();
	MoveGeneration::init();
}

// set some data (origin/target square, colour bitboard, and piece bitboard) of the two moves
// that are made during a castle move (to the engine, it is like the rook and the king both move on the same turn)
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

	// sets the origin/target square, colour bitboard, and piece bitboard of the two "half" moves
    setCastleMoveData(md, &kingMove, &rookMove);

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
	else // otherwise, make the move as normal
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
			Bitboard capturedPieceBB = moveData->side == SIDE_WHITE ? (BB::southOne(targetBB)) : (BB::northOne(targetBB));

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

// returns the coordinate of the king by number (0-63)
Byte Board::computeKingSquare(Bitboard kingBB)
{
	if (kingBB)
		return BB::getLSB(kingBB);

	return 0;
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
		Bitboard northWestAttacksBB = BB::northWestOne(opPawnsBB & BB::fileClear[BB::FILE_A]);
		Bitboard northEastAttacksBB = BB::northEastOne(opPawnsBB & BB::fileClear[BB::FILE_H]);
		if (BB::boardSquares[square] & (northWestAttacksBB | northEastAttacksBB))
			return true;
	}
	else
	{
		Bitboard southWestAttacksBB = BB::southWestOne(opPawnsBB & BB::fileClear[BB::FILE_A]);
		Bitboard southEastAttacksBB = BB::southEastOne(opPawnsBB & BB::fileClear[BB::FILE_H]);
		if (BB::boardSquares[square] & (southWestAttacksBB | southEastAttacksBB))
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
// the function uses pointer arguments to return data about who the attacker is (how many centipawns they're worth, and the piece bitboard they belong to)
void Board::getLeastValuableAttacker(Byte square, Colour attackingSide, int* pieceValue, Bitboard** pieceBB, Bitboard* attackingPiecesBB)
{
	Bitboard opPawnsBB = attackingSide == SIDE_WHITE ? currentPosition.whitePawnsBB : currentPosition.blackPawnsBB;

	// this gets all of the diagonal attacks of the pawns depending on the side attacking. by clearing the A and H files,
	// we are preventing the "attack" from overflowing into the file on the opposite end of the board
	if (attackingSide == SIDE_WHITE)
	{
		Bitboard northWestAttacksBB = BB::northWestOne(opPawnsBB & BB::fileClear[BB::FILE_A]);
		Bitboard northEastAttacksBB = BB::northEastOne(opPawnsBB & BB::fileClear[BB::FILE_H]);

		*attackingPiecesBB = (northWestAttacksBB | northEastAttacksBB) & BB::boardSquares[square];
		if (*attackingPiecesBB)
		{
			// by shifting the attacks bitboards in the opposite direction, we can get
			// the bitboard that contains the location of all the pawns attacking the square
			*attackingPiecesBB = BB::southEastOne(northWestAttacksBB) | BB::southWestOne(northEastAttacksBB);
			*pieceValue = Eval::PAWN_VALUE;
			*pieceBB = &currentPosition.whitePawnsBB;
			return;
		}
	}
	else
	{
		Bitboard southWestAttacks = BB::southWestOne(opPawnsBB & BB::fileClear[BB::FILE_A]);
		Bitboard southEastAttacks = BB::southEastOne(opPawnsBB & BB::fileClear[BB::FILE_H]);

		// by shifting the attacks bitboards in the opposite direction, we can get
		// the bitboard that contains the location of all the pawns attacking the square
		*attackingPiecesBB = (southWestAttacks | southEastAttacks) & BB::boardSquares[square];
		if (*attackingPiecesBB)
		{
			*attackingPiecesBB = BB::northEastOne(southWestAttacks) | BB::northWestOne(southEastAttacks);
			*pieceValue = Eval::PAWN_VALUE;
			*pieceBB = &currentPosition.blackPawnsBB;
			return;
		}
	}

	Bitboard* opKnightsBB = attackingSide == SIDE_WHITE ? &currentPosition.whiteKnightsBB : &currentPosition.blackKnightsBB;
	*attackingPiecesBB = MoveGeneration::knightLookupTable[square] & *opKnightsBB;
	if (*attackingPiecesBB)
	{
		*pieceValue = Eval::KNIGHT_VALUE;
		*pieceBB = opKnightsBB;
		return;
	}

	Bitboard friendlyPieces = attackingSide == SIDE_WHITE ? currentPosition.blackPiecesBB : currentPosition.whitePiecesBB;

	Bitboard* opBishopsBB = attackingSide == SIDE_WHITE ? &currentPosition.whiteBishopsBB : &currentPosition.blackBishopsBB;
	Bitboard bishopMovesBB = MoveGeneration::computePseudoBishopMoves(square, currentPosition.occupiedBB, friendlyPieces);
	*attackingPiecesBB = bishopMovesBB & *opBishopsBB;
	if (*attackingPiecesBB)
	{
		*pieceValue = Eval::BISHOP_VALUE;
		*pieceBB = opBishopsBB;
		return;
	}

	Bitboard* opRooks = attackingSide == SIDE_WHITE ? &currentPosition.whiteRooksBB : &currentPosition.blackRooksBB;
	Bitboard rookMovesBB = MoveGeneration::computePseudoRookMoves(square, currentPosition.occupiedBB, friendlyPieces);
	*attackingPiecesBB = rookMovesBB & *opRooks;
	if (*attackingPiecesBB)
	{
		*pieceValue = Eval::ROOK_VALUE;
		*pieceBB = opRooks;
		return;
	}

	Bitboard* opQueens = attackingSide == SIDE_WHITE ? &currentPosition.whiteQueensBB : &currentPosition.blackQueensBB;
	*attackingPiecesBB = (rookMovesBB | bishopMovesBB) & *opQueens;
	if (*attackingPiecesBB)
	{
		*pieceValue = Eval::QUEEN_VALUE;
		*pieceBB = opQueens;
		return;
	}

	Bitboard* opKingsBB = attackingSide == SIDE_WHITE ? &currentPosition.whiteKingBB : &currentPosition.blackKingBB;
	*attackingPiecesBB = MoveGeneration::kingLookupTable[square] & *opKingsBB;
	if (*attackingPiecesBB)
	{
		*pieceValue = Eval::KING_VALUE;
		*pieceBB = opKingsBB;
		return;
	}
}

// if the move made generated an en passant square, set the current en passant square for the current position
void Board::setEnPassantSquares(MoveData* moveData)
{
	// default to no en passant squares being set
	currentPosition.enPassantSquare = 0;

	// the below if statements check to see if the move would have set an en passant square by checking if the piece
	// moved was a pawn, and if so, if it moved 2 spaces (8 tiles * 2 = 16). if it passes, it will set the coordinate
	// of the en passant square to the correct value for the current position

	if (moveData->pieceBB == &currentPosition.whitePawnsBB && moveData->targetSquare - moveData->originSquare == 16)
		currentPosition.enPassantSquare = moveData->targetSquare - 8;
	else if (moveData->pieceBB == &currentPosition.blackPawnsBB && moveData->originSquare - moveData->targetSquare == 16)
		currentPosition.enPassantSquare = moveData->targetSquare + 8;
}

// adds the current position's zobrist key into the game's position history 
void Board::insertMoveIntoHistory(short ply)
{
	mZobristKeyHistory[ply] = mCurrentZobristKey;
}

// removes the current position's zobrist key from the game's position history
void Board::deleteMoveFromHistory(short ply)
{
	mZobristKeyHistory[ply] = 0;
}

/* 
    using the information in the MoveData struct, this functions performs the following:
		update the bitboards affected in the move
		check legality of the move (i.e. if it would result in a check)
		update castle privileges
		set any en passant squares, update the fifty move counter, and change the side to move
		insert the move into the position history (unless it was one of the two castle moves making up the whole move)
*/
bool Board::makeMove(MoveData* moveData)
{
	// check to see if the move was a castling move
	if (moveData->moveType == MoveData::EncodingBits::SHORT_CASTLE || moveData->moveType == MoveData::EncodingBits::LONG_CASTLE)
	{
		if (!makeCastleMove(moveData))
			return false;
	}
	else
	{
		updateBitboardWithMove(moveData);

		// checking to see if the king is in check (if it is, then the move would be illegal)
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

	// if the move was legal, then account for all the changes to the current position that the move could have had
	// en passant squares, castle privileges, drawing conditions, side to move, etc

	setEnPassantSquares(moveData);

	// update the castle privileges after the move
	currentPosition.castlePrivileges &= ~moveData->castlePrivilegesRevoked;

	// when a castle move occurs, two moves are actually made (meaning that this function is called twice)
	// however, one of those moves is designated as a castle half move, meaning that it doesn't cause any changes
	// to the state of the current position, other than updating the bitboards and the current position's zobrist
	// key. the other move, however, is not designated as such, and will cause the below if statement to pass,
	// thereby updating all the rest of the data it needs to
	if (moveData->moveType != MoveData::EncodingBits::CASTLE_HALF_MOVE)
	{
		// update the fifty move counter drawing condition
		moveData->fiftyMoveCounter = currentPosition.fiftyMoveCounter;

		// reset the fifty move counter if there was a capture or a pawn advance, otherwise increment it
		if (moveData->capturedPieceBB || moveData->pieceBB == &currentPosition.whitePawnsBB || moveData->pieceBB == &currentPosition.blackPawnsBB)
			currentPosition.fiftyMoveCounter = 0;
		else
			currentPosition.fiftyMoveCounter++;

		currentPosition.sideToMove = !currentPosition.sideToMove;
		mCurrentZobristKey = ZobristKey::update(mCurrentZobristKey, &currentPosition, moveData);
		insertMoveIntoHistory(++mPly);
	}
	else
		mCurrentZobristKey = ZobristKey::update(mCurrentZobristKey, &currentPosition, moveData);

	return true;
}

/*
	using the information in the MoveData struct, this functions takes a move back. It does so by:
		undoing any pawn promotions
		updating the bitboards that were affected in the move
		reseting en passant squares, castle privileges, side to move, and fifty move counter by using the values stored in the move data
		deleting the move from the position history
*/
bool Board::unmakeMove(MoveData* moveData, bool positionUpdated)
{
	// if the move we're unmaking was a castle move, then we want to undo the castle move by calling the make castle move function
	// this function checks to see if we're unmaking the castle move or not, and so it handles all the necessary function calls
	// to properly undo the castle move
	if (moveData->moveType == MoveData::EncodingBits::SHORT_CASTLE || moveData->moveType == MoveData::EncodingBits::LONG_CASTLE)
		makeCastleMove(moveData);
	else
	{
		// if a promotion occured during the move, then undo whatever promotion occured
		undoPromotion(moveData);

		// update the bitboards so that they are as they were before the move was made this can
		// simply be done by calling the exact same function that made the move in the first place
		updateBitboardWithMove(moveData);
	}

	// as described in Board::makeMove, a castling move consists of two actual pieces moving
	// one of them will be designated as a move that shouldn't cause updates (if it has moveType = CASTLE_HALF_MOVE)
	// as such, should a castle move have occured, the below if statement will only pass once, even though
	// resetting the castle move will cause two separate moves
	if (moveData->moveType != MoveData::EncodingBits::CASTLE_HALF_MOVE && positionUpdated)
	{
		// reset the zobrist key to the previus position's
		mCurrentZobristKey = mZobristKeyHistory[mPly - 1];

		// reset all of the data as it was before the move was made
		currentPosition.enPassantSquare   = moveData->enPassantSquare;
		currentPosition.castlePrivileges ^= moveData->castlePrivilegesRevoked;
		currentPosition.fiftyMoveCounter  = moveData->fiftyMoveCounter;
		currentPosition.sideToMove 		  = !currentPosition.sideToMove;

		// remove the current position from the position history
		deleteMoveFromHistory(mPly--);
	}

	return true;
}

// replaces the promoted pawn back to a pawn and removes the piece the pawn promoted to from its respective bitboard
void Board::undoPromotion(MoveData* moveData)
{
	// places the pawn back to its previous position
	if (moveData->moveType == MoveData::EncodingBits::BISHOP_PROMO || moveData->moveType == MoveData::EncodingBits::ROOK_PROMO ||
		moveData->moveType == MoveData::EncodingBits::KNIGHT_PROMO || moveData->moveType == MoveData::EncodingBits::QUEEN_PROMO)
	{
		if (moveData->side == SIDE_WHITE) currentPosition.whitePawnsBB ^= BB::boardSquares[moveData->targetSquare];
		else							  currentPosition.blackPawnsBB ^= BB::boardSquares[moveData->targetSquare];
	}
	else
		return;

	// removes the piece that the pawn promoted to
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

// this function deletes a pawn and adds in the piece that the pawn promoted to at the pawn's position
void Board::promotePiece(MoveData* md, MoveData::EncodingBits promoteTo)
{
	// this will set the move's movetype to whatever piece it is promoting to
	// this is useful for when we want to unpromote a piece during a move search
	md->setMoveType(promoteTo);

	if (md->side == SIDE_WHITE)
	{
		// reset the bit that the pawn is on (thus removing it from the bitboard)
		currentPosition.whitePawnsBB ^= BB::boardSquares[md->targetSquare];

		// add the piece that the pawn promoted to to the respective bitboard 
		if		(promoteTo == MoveData::EncodingBits::QUEEN_PROMO)	currentPosition.whiteQueensBB  |= BB::boardSquares[md->targetSquare];
		else if (promoteTo == MoveData::EncodingBits::ROOK_PROMO)	currentPosition.whiteRooksBB   |= BB::boardSquares[md->targetSquare];
		else if (promoteTo == MoveData::EncodingBits::BISHOP_PROMO) currentPosition.whiteBishopsBB |= BB::boardSquares[md->targetSquare];
		else if (promoteTo == MoveData::EncodingBits::KNIGHT_PROMO) currentPosition.whiteKnightsBB |= BB::boardSquares[md->targetSquare];
	}
	else
	{
		// reset the bit that the pawn is on (thus removing it from the bitboard)
		currentPosition.blackPawnsBB ^= BB::boardSquares[md->targetSquare];

		// add the piece that the pawn promoted to to the respective bitboard 
		if		(promoteTo == MoveData::EncodingBits::QUEEN_PROMO)	currentPosition.blackQueensBB  |= BB::boardSquares[md->targetSquare];
		else if (promoteTo == MoveData::EncodingBits::ROOK_PROMO)	currentPosition.blackRooksBB   |= BB::boardSquares[md->targetSquare];
		else if (promoteTo == MoveData::EncodingBits::BISHOP_PROMO) currentPosition.blackBishopsBB |= BB::boardSquares[md->targetSquare];
		else if (promoteTo == MoveData::EncodingBits::KNIGHT_PROMO) currentPosition.blackKnightsBB |= BB::boardSquares[md->targetSquare];
	}
}