#include <sstream>

#include "utils.h"

// splits a string into chunks based off of a delimiter, and stores them into the vector passed in by reference
void splitString(const std::string& string, std::vector<std::string>& vec, char delimiter)
{
	std::stringstream ss(string);
	std::string splitString;
	while (std::getline(ss, splitString, delimiter))
		vec.push_back(splitString);
}

/*
	the following two functions serve to count the number of set bits in a 64 bit number in an efficient matter
	details about their implentation can be found at https://www.geeksforgeeks.org/count-set-bits-in-an-integer/
*/

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