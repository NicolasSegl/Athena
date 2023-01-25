#pragma once

// the full FEN string (containing the castle privileges, side to move, etc) that represents the start of a chess game
const char FEN_STARTING_STRING[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

// little endian file mapping to chess coordinate system
namespace ChessCoord
{
    enum CC
    {
        A1, B1, C1, D1, E1, F1, G1, H1,
        A2, B2, C2, D2, E2, F2, G2, H2,
        A3, B3, C3, D3, E3, F3, G3, H3,
        A4, B4, C4, D4, E4, F4, G4, H4,
        A5, B5, C5, D5, E5, F5, G5, H5,
        A6, B6, C6, D6, E6, F6, G6, H6,
        A7, B7, C7, D7, E7, F7, G7, H7,
        A8, B8, C8, D8, E8, F8, G8, H8,
    };
}

// this namespace defines some constants for some ASCII codes
namespace ASCII
{
    const int NUMBER_ONE_CODE  = 49;
    const int NUMBER_NINE_CODE = 57;
    const int LETTER_A_CODE    = 97;
    const int TAB_CODE         = 0x9;
}