#include <chrono>
#include <random>
#include <time.h>
#include <iostream>

#include "ChessGame.h"
#include "UCI.h"

// I LOVE YOU CHARLOTTE <3 I LOVE YOU MORE!!!!

int main()
{
	std::srand(time(NULL));

	UCI::run();
	return 0;
}

// TODO

// perft
// parallel search
// remove the ChessPosition class? doesn't really do anything ?
// insufficient material
// get the engine to never use up more time than it has. keep a clock and every loop of negamax see if the time we have alloted it is up
// change the MoveData::Encoding bits flag so it's actually a bit flag
// make structs/classes smaller in size

// and ENDGAME PLAY
// test the fifty move rule
// switch statement Board::promoteTo
// pawn structure https://www.chessprogramming.org/Pawn_Pattern_and_Properties
// move ordering
//	consider also pawn promotions/advances
// futility pruning

// make all structures as small as possible. make movedata use bits set to get data
// refactor everything so that all functions are short and do only one thing, and that methods are in appropriate classes
// for the above maybe look to other chess engines online. some have classes only for Search and one for making Moves, etc
// extensions
// search windows
// one simple extension is to simply increase depth by 1 when under check
// reductions