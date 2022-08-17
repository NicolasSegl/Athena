#include <sstream>

#include "utils.h"

void splitString(const std::string& string, std::vector<std::string>& vec, char toSplitCharacter)
{
	std::stringstream ss(string);
	std::string splitString;
	while (std::getline(ss, splitString, toSplitCharacter))
		vec.push_back(splitString);
}

// calculate the number of set bits by lookup. code mostly provided by geeksforgeeks at https://www.geeksforgeeks.org/count-set-bits-in-an-integer/
int bitsSetTable256[256];

void initBitsSetTable()
{
	for (int i = 0; i < 256; i++)
		bitsSetTable256[i] = (i & 1) + bitsSetTable256[i / 2];
}

int countSetBits64(uint64_t number)
{
	return bitsSetTable256[number & 0xff]	      + bitsSetTable256[(number >> 8) & 0xff]  +
		   bitsSetTable256[(number >> 16) & 0xff] + bitsSetTable256[(number >> 24) & 0xff] +
		   bitsSetTable256[(number >> 32) & 0xff] + bitsSetTable256[(number >> 40) & 0xff] +
		   bitsSetTable256[(number >> 48) & 0xff] + bitsSetTable256[(number >> 56) & 0xff];
}