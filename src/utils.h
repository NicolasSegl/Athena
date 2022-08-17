#pragma once

#include <string>
#include <vector>
#include <cinttypes>

void splitString(const std::string& string, std::vector<std::string>& vec, char toSplitCharacter);
int countSetBits64(uint64_t number);
void initBitsSetTable();