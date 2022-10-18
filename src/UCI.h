#pragma once

#include <string>
#include <vector>

#include "ChessGame.h"

namespace UCI
{
	void run();
	void processCommand(const std::string& commandString);
};