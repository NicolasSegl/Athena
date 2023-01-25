#pragma once

#include "ZobristKey.h"

// this declaration is necessary to prevent circular including
class Board;

namespace Outcomes
{
	bool isDraw(Board* boardPtr);
};