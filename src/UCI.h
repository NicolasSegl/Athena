#pragma once

#include <string>
#include <vector>

#include "ChessGame.h"

// defines the functions that allow Athena to interact with the UCI interface
namespace UCI
{
	void run();
	void processCommand(const std::string& commandString);
};