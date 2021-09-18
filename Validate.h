#pragma once
#include <vector>
struct Cell;
class Puzzle;

struct RegionData {
  std::vector<Cell*> invalidElements;
  std::vector<Cell*> veryInvalidElements;
  std::vector<std::tuple<Cell*, Cell*>> negations;

  inline bool Valid() {
    return invalidElements.size() == 0 && veryInvalidElements.size() == 0;
  }
};

class Validator {
public:
  // Determines if the current grid state is solvable. Modifies the puzzle element with:
  // valid: Whether or not the puzzle is valid
  // invalidElements: Symbols which are invalid (for the purpose of negating / flashing)
  // negations: Negation pairs (for the purpose of darkening)
  static void Validate(Puzzle& puzzle, bool quick = false);

  // Determines whether or not a particular region is valid or not, including negation symbols.
  // If quick is true, exits after the first invalid element is found (small performance gain)
  // This function applies negations to all "very invalid elements", i.e. elements which cannot become
  // valid by another element being negated. Then, it passes off to regionCheckNegations2,
  // which attempts to apply any remaining negations to any other invalid elements.
  static RegionData ValidateRegion(const Puzzle& puzzle, const Region& region, bool quick = false);

private:
  // Recursively matches negations and invalid elements from the grid. Note that this function
  // doesn't actually modify the two lists, it just iterates through them with index/index2.
  static RegionData RegionCheckNegations2(const Puzzle& puzzle, const Region& region, const std::vector<Cell*>& negationSymbols, const std::vector<Cell*>& invalidElements, int index = 0, int index2 = 0) {
    return RegionData();
  }

  // Checks if a region is valid. This does not handle negations -- we assume that there are none.
  // Note that this function needs to always ask the puzzle for the current contents of the cell,
  // since the region is only coordinate locations, and might be modified by regionCheckNegations2
  // @Performance: This is a pretty core function to the solve loop.
  static RegionData RegionCheck(const Puzzle& puzzle, const Region& region, bool quick = false);
};