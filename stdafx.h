#pragma once

#ifdef _DEBUG
#undef assert
#define assert(cond) \
{ \
  if (!(cond)) { \
    *static_cast<volatile int*>(nullptr) = 1; \
  } \
}
#else
#define assert(cond)
#endif

#include "Polyominos.h"
#include "Puzzle.h"
#include "Random.h"
#include "Solve.h"
#include "StdLib.h"
#include "Utilities.h"
#include "Validate.h"

using namespace std;

struct RegionData {
  RegionData(int size) {
    invalidElements = Vector<Cell*>(size);
    veryInvalidElements = Vector<Cell*>(size);
    negations = Vector<pair<Cell*, Cell*>>(size);
  }
  Vector<Cell*> invalidElements;
  Vector<Cell*> veryInvalidElements;
  Vector<pair<Cell*, Cell*>> negations;

  bool Valid() {
    return invalidElements.Size() == 0 && veryInvalidElements.Size() == 0;
  }
};
