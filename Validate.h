#pragma once
#include <vector>
struct Cell;
struct Region;
class Puzzle;

struct RegionData {
  std::vector<Cell*> invalidElements;
  std::vector<Cell*> veryInvalidElements;
  std::vector<Cell*> negations;

  inline bool Valid() {
    return invalidElements.size() == 0 && veryInvalidElements.size() == 0;
  }
};

class Validator {
public:
  void Validate(Puzzle& puzzle, bool quick = false);

private:
  RegionData ValidateRegion(Puzzle& puzzle, const Region& region, bool quick = false);
};