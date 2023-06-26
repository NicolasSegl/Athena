#include <chrono>
#include <random>
#include <time.h>
#include <iostream>

#include "ChessGame.h"
#include "UCI.h"

int main()
{
	// seed the random function
	std::srand(1351241596345);

	// listen to UCI commands
	UCI::run();

	return 0;
}