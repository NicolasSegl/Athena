#pragma once

#include "Board.h"
#include "Constants.h"
#include "MoveData.h"

struct UserInput
{
	enum class InputType
	{
		SquareSelect,
		GameClose,
		UndoMove,
		Nothing,
	};

	InputType inputType;
	int squareLoc;
};

class ChessGUI
{
private:
    struct guiSquareData
    {
        sf::RectangleShape rect;
        sf::Color ogColour;

        void resetColour() { rect.setFillColor(ogColour); }
    };

	sf::RenderWindow mWindow;
    guiSquareData mBoardSquares[64];

	sf::Sprite mPieceSprites[12];

	int mWindowWidth, mWindowHeight, mSquareSize;;
	sf::Color mDarkColour, mLightColour, mSelectColour;

	int mLastSelectedSquare;

	void drawPiece(sf::Vector2f pos, PieceTypes::ColourPieceType spriteType);
	void createSquareSprites();

public:
	ChessGUI();

	void init(int wWidth, int wHeight);

	UserInput getUserInput(); 
	void updateBoard(Board* board);
	void setMoveColours(MoveData* md);
	void setSelectedSquare(Byte square);
	void unselectSelectedSquare();
	
	void resetAllColours();
};

