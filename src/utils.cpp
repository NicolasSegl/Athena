#include <sstream>

#include "utils.h"

void splitString(const std::string& string, std::vector<std::string>& vec, char toSplitCharacter)
{
	std::stringstream ss(string);
	std::string splitString;
	while (std::getline(ss, splitString, toSplitCharacter))
		vec.push_back(splitString);
}