#include "ChessGUI.h"
#include "Bitboard.h"

#include <iostream>

const int SPRITE_SIZE = 80; // pixels

ChessGUI::ChessGUI()
{
	mDarkColour			  = sf::Color(242, 206, 162);
	mLightColour		  = sf::Color(245, 245, 245);
	mSelectColour		  = sf::Color(196, 228, 157);

	mLastSelectedSquare = -1;

	static sf::Image spriteSheetImage;
	if (!spriteSheetImage.loadFromFile("pieceSpriteSheet.png"))
		std::cout << "Sprite Sheet could not load\n";

	spriteSheetImage.createMaskFromColor(sf::Color(127, 127, 127));

	static sf::Texture spriteSheetTexture;
	spriteSheetTexture.loadFromImage(spriteSheetImage);
	spriteSheetTexture.setSmooth(true);

	for (int spriteIndex = 0; spriteIndex < 12; spriteIndex++)
	{
		mPieceSprites[spriteIndex].setTexture(spriteSheetTexture);

		if (spriteIndex < 6)
			mPieceSprites[spriteIndex].setTextureRect(sf::IntRect(spriteIndex * SPRITE_SIZE + spriteIndex, 0, SPRITE_SIZE, SPRITE_SIZE));
		else
			mPieceSprites[spriteIndex].setTextureRect(sf::IntRect(spriteIndex * SPRITE_SIZE + spriteIndex - (SPRITE_SIZE * 6 + 6),
																  SPRITE_SIZE + 1, SPRITE_SIZE, SPRITE_SIZE));
	}
}

void ChessGUI::createSquareSprites()
{
	bool darkColour = true;
	for (int square = 63; square >= 0; square--)
	{
		mBoardSquares[square].rect.setSize(sf::Vector2f(mSquareSize, mSquareSize));
		mBoardSquares[square].rect.setPosition(sf::Vector2f((7 - (square % 8)) * mWindowHeight / 8, square / 8 * mWindowHeight / 8));
		mBoardSquares[square].rect.setOutlineThickness(1);

		if (darkColour)
			mBoardSquares[square].ogColour = mDarkColour;
		else
			mBoardSquares[square].ogColour = mLightColour;

		mBoardSquares[square].rect.setFillColor(mBoardSquares[square].ogColour);
		darkColour = !darkColour;

		if (square % 8 == 0)
			darkColour = !darkColour;
	}
}

void ChessGUI::init(int wWidth, int wHeight)
{
	mWindowWidth  = wWidth;
	mWindowHeight = wHeight;
	mWindow.create(sf::VideoMode(mWindowWidth, mWindowHeight), "Chess Engine", sf::Style::Titlebar | sf::Style::Close);

	mSquareSize = mWindowHeight / 8;

	// so that the sprites are never larger than the squares (only if special window dimensions are inputted)
	while (SPRITE_SIZE > mSquareSize)
		for (int i = 0; i < 12; i++)
			mPieceSprites[i].setScale(sf::Vector2f(0.9, 0.9));

	createSquareSprites();
}

UserInput ChessGUI::getUserInput()
{
	UserInput input;

	while (mWindow.isOpen())
	{
		sf::Event event;
		while (mWindow.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				input.inputType = UserInput::InputType::GameClose;
				mWindow.close();
				return input;
			}
			else if (event.type == sf::Event::MouseButtonPressed)
			{
				int mx = sf::Mouse::getPosition(mWindow).x;
				int my = mWindowHeight - sf::Mouse::getPosition(mWindow).y;

				if (event.mouseButton.button == sf::Mouse::Left)
				{
					input.inputType = UserInput::InputType::SquareSelect;
					input.squareLoc = my / mSquareSize * 8 + mx / mSquareSize;
					if (input.squareLoc < 0 || input.squareLoc > 63)
					{
						input.inputType = UserInput::InputType::Nothing;
						return input;
					}

					return input;
				}
			}
			else if (event.type == sf::Event::KeyPressed)
			{
				input.inputType = UserInput::InputType::UndoMove;
				return input;
			}
		}
	}
}

void ChessGUI::unselectSelectedSquare()
{
    if (mLastSelectedSquare >= 0)
    {
        mBoardSquares[63 - mLastSelectedSquare].resetColour();
        mLastSelectedSquare = -1;
    }
}

void ChessGUI::setSelectedSquare(Byte square)
{
	if (mLastSelectedSquare >= 0)
	{
		mBoardSquares[63 - mLastSelectedSquare].resetColour();
		mLastSelectedSquare = square;
	}

	mBoardSquares[63 - square].rect.setFillColor(mSelectColour);
    mLastSelectedSquare = square;
}

void ChessGUI::drawPiece(sf::Vector2f pos, PieceTypes::ColourPieceType spriteType)
{
	mPieceSprites[spriteType].setPosition(pos);
	mWindow.draw(mPieceSprites[spriteType]);
}

void ChessGUI::setMoveColours(MoveData* md)
{
	static int oldMoveSquares[2] = { -1, -1 };

	// if the first move has been made and oldColours have been initialized, change them back after the move !
	if (oldMoveSquares[0] > -1)
	{
		mBoardSquares[oldMoveSquares[0]].resetColour();
		mBoardSquares[oldMoveSquares[1]].resetColour();
	}

	oldMoveSquares[0] = 63 - md->originSquare;
	oldMoveSquares[1] = 63 - md->targetSquare;

	mBoardSquares[63 - md->targetSquare].rect.setFillColor(mSelectColour);
	mBoardSquares[63 - md->originSquare].rect.setFillColor(mSelectColour);
	mLastSelectedSquare = -1;
}

void ChessGUI::resetAllColours()
{
	for (int square = 0; square < 64; square++)
		mBoardSquares[square].resetColour();
}

void ChessGUI::updateBoard(Board* board)
{
	mWindow.clear();

	for (int square = 0; square < 64; square++)
		mWindow.draw(mBoardSquares[square].rect);

	// draw all of the pieces on the board
	for (int bit = 63; bit >= 0; bit--)
	{
		// currently only works for SPRITE_SIZE = 80
		sf::Vector2f translatedPos(bit % 8 * mSquareSize + (mSquareSize - SPRITE_SIZE) / 2,
								   (7 - bit / 8) * mSquareSize + (mSquareSize - SPRITE_SIZE) / 2); // they all start top left
		// try for bit = 0

		if		(board->currentPosition.whitePawnsBB & BB::boardSquares[bit])	drawPiece(translatedPos, PieceTypes::WHITE_PAWN);
		else if (board->currentPosition.whiteKingBB & BB::boardSquares[bit])	drawPiece(translatedPos, PieceTypes::WHITE_KING);
		else if (board->currentPosition.whiteRooksBB & BB::boardSquares[bit])   drawPiece(translatedPos, PieceTypes::WHITE_ROOK);
		else if (board->currentPosition.whiteBishopsBB & BB::boardSquares[bit]) drawPiece(translatedPos, PieceTypes::WHITE_BISHOP);
		else if (board->currentPosition.whiteQueensBB & BB::boardSquares[bit])  drawPiece(translatedPos, PieceTypes::WHITE_QUEEN);
		else if (board->currentPosition.whiteKnightsBB & BB::boardSquares[bit]) drawPiece(translatedPos, PieceTypes::WHITE_KNIGHT);

		if		(board->currentPosition.blackPawnsBB & BB::boardSquares[bit])	drawPiece(translatedPos, PieceTypes::BLACK_PAWN);
		else if (board->currentPosition.blackKingBB & BB::boardSquares[bit])	drawPiece(translatedPos, PieceTypes::BLACK_KING);
		else if (board->currentPosition.blackRooksBB & BB::boardSquares[bit])   drawPiece(translatedPos, PieceTypes::BLACK_ROOK);
		else if (board->currentPosition.blackBishopsBB & BB::boardSquares[bit]) drawPiece(translatedPos, PieceTypes::BLACK_BISHOP);
		else if (board->currentPosition.blackQueensBB & BB::boardSquares[bit])  drawPiece(translatedPos, PieceTypes::BLACK_QUEEN);
		else if (board->currentPosition.blackKnightsBB & BB::boardSquares[bit]) drawPiece(translatedPos, PieceTypes::BLACK_KNIGHT);
	}

	mWindow.display();
}
