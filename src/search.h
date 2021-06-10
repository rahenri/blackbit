#pragma once

#include "board.h"
#include "move.h"

struct Search {
  static Move ComputerPlay(Board &board, int depth, bool post);
};
