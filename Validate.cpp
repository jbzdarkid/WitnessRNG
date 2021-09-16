#include "Validate.h"
#include "Puzzle.h"
#include "Utilities.h"

using namespace std;

void Validator::Validate(Puzzle& puzzle, bool quick) {
  console.log("Validating", puzzle);
  puzzle._valid = true; // Assume valid until we find an invalid element

  bool needsRegions = false;
  Region monoRegion = Region(2*puzzle._width + 1);
  // These two are both used by validateRegion, so they are saved on the puzzle itself.
  puzzle._hasNegations = false;
  puzzle._hasPolyominos = false;

  // Validate gap failures as an early exit.
  for (int x=0; x<2*puzzle._width+1; x++) {
    for (int y=0; y<2*puzzle._height+1; y++) {
      Cell cell = puzzle._grid[x][y];
      // if (cell == nullptr) continue;
      if (!needsRegions && cell.type != "line" && cell.type != "triangle") needsRegions = true;
      if (cell.type == "nega") puzzle._hasNegations = true;
      if (cell.type == "poly" || cell.type == "ylop") puzzle._hasPolyominos = true;
      if (cell.line > LINE_NONE) {
        if (cell.gap > GAP_NONE) {
          console.log("Solution line goes over a gap at", x, y);
          puzzle._valid = false;
          if (quick) return;
        }
        if ((cell.dot == DOT_BLUE && cell.line == LINE_YELLOW) ||
            (cell.dot == DOT_YELLOW && cell.line == LINE_BLUE)) {
          console.log("Incorrectly covered dot: Dot is", cell.dot, "but line is", cell.line);
          puzzle._valid = false;
          if (quick) return;
        }
      } else {
        monoRegion.SetCell(x, y);
      }
    }
  }

  puzzle._invalidElements.clear();
  puzzle._negations.clear();
  vector<Region> regions;
  if (needsRegions) {
    regions = puzzle.GetRegions();
  } else {
    regions = { monoRegion };
  }
  console.log("Found", regions.size(), "region(s)");
  console.debug(regions);

  if (false /*puzzle.settings.CUSTOM_MECHANICS*/) {
  } else {
    for (auto region : regions) {
      auto regionData = ValidateRegion(puzzle, region, quick);
      console.log("Region valid:", regionData.Valid());
      Append(puzzle._negations, regionData.negations);
      Append(puzzle._invalidElements, regionData.invalidElements);
      Append(puzzle._veryInvalidElements, regionData.veryInvalidElements);
      puzzle._valid = puzzle._valid && regionData.Valid();
      if (quick && !puzzle._valid) return;
    }
  }
  console.log("Puzzle has", puzzle._invalidElements.size(), "invalid elements");
}

RegionData Validator::ValidateRegion(Puzzle& puzzle, const Region& region, bool quick) {
  return RegionData();
}
