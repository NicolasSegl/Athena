#pragma once

#include <string>

typedef uint8_t  Byte;
typedef uint16_t DoubleByte;
typedef bool Colour;

namespace PieceTypes
{
	enum ColourPieceType
	{
		WHITE_PAWN,
		WHITE_ROOK,
		WHITE_BISHOP,
		WHITE_QUEEN,
		WHITE_KNIGHT,
		WHITE_KING,

		BLACK_PAWN,
		BLACK_ROOK,
		BLACK_BISHOP,
		BLACK_QUEEN,
		BLACK_KNIGHT,
		BLACK_KING,
	};

	enum Types
	{
		KING,
		QUEEN,
		ROOK,
		BISHOP,
		KNIGHT,
		PAWN,
		NONE,
	};
}

namespace AthenaConstants
{
	const int MAX_PLY = 15;

	// this is a value used to properly assign move scores
	// we use this value so that we can use smaller numbers to describe move scores given by certain captures
	// i.e. so a pawn capture would have a value of 1 + 100 = 101, whereas a killer move heuristic move would have 100 - 10 = 90
	// which would give the proper ordering to the capture move
	const Byte MVV_LVA_OFFSET = 100;

	const int MAX_KILLER_MOVES = 2;
	const Byte KILLER_MOVE_SCORE = 10;
}

namespace Evaluation
{
	const int CHECKMATE_SCORE = 1000000;
}

namespace FEN
{
	const std::string start = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

	namespace Fields
	{
		const int piecePlacement   = 0;
		const int sideToPlay       = 1;
		const int castlePrivileges = 2;
		const int enPassantSquare  = 3;
		const int fiftyMoveCounter = 4;
		const int totalMoves	   = 5;
	}
}

namespace ASCII
{
	const int NUMBER_ONE_CODE = 49;
	const int LETTER_A_CODE   = 97;
}