#pragma once

#include <string>
#include <vector>

#include "ChessGame.h"

class UCI
{
private:
	ChessGame mCG;

	void respondUCI();
	void respondIsReady();
	void respondPosition(const std::vector<std::string>& commandVec);
	void respondGo(const std::vector<std::string>& commandVec);

	void processCommand(const std::string& commandString);

public:
	void run();
};