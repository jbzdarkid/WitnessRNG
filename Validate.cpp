#include <unordered_map>

#include "Polyominos.h"
#include "Puzzle.h"
#include "Utilities.h"
#include "Validate.h"

using namespace std;

void Validator::Validate(Puzzle& puzzle, bool quick) {
  console.log("Validating", puzzle);
  puzzle._valid = true; // Assume valid until we find an invalid element

  bool needsRegions = false;
  Region monoRegion = Region(puzzle._width);
  // These two are both used by validateRegion, so they are saved on the puzzle itself.
  puzzle._hasNegations = false;
  puzzle._hasPolyominos = false;

  // Validate gap failures as an early exit.
  for (int x=0; x<puzzle._width; x++) {
    for (int y=0; y<puzzle._height; y++) {
      Cell cell = puzzle._grid[x][y]; // Copy is probably cheaper here, since we reference some data more than once.
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

  for (auto region : regions) {
    auto regionData = ValidateRegion(puzzle, region, quick);
    console.log("Region valid:", regionData.Valid());
    Append(puzzle._negations, regionData.negations);
    Append(puzzle._invalidElements, regionData.invalidElements);
    Append(puzzle._veryInvalidElements, regionData.veryInvalidElements);
    puzzle._valid = puzzle._valid && regionData.Valid();
    if (quick && !puzzle._valid) return;
  }
  console.log("Puzzle has", puzzle._invalidElements.size(), "invalid elements");
}

RegionData Validator::ValidateRegion(const Puzzle& puzzle, const Region& region, bool quick) {
  if (!puzzle._hasNegations) return RegionCheck(puzzle, region, quick);

  // Get a list of negation symbols in the grid, and set them to "nonce"
  vector<Cell*> negationSymbols;
  for (const auto [x, y] : region.cells) {
    Cell* cell = &puzzle._grid[x][y];
    if (cell->type == "nega") {
      cell->type = "nonce";
      negationSymbols.push_back(cell);
    }
  }
  console.debug("Found", negationSymbols.size(), "negation symbols");
  if (negationSymbols.size() == 0) {
    // No negation symbols in this region. Note that there must be negation symbols elsewhere
    // in the puzzle, since puzzle.hasNegations was true.
    return RegionCheck(puzzle, region, quick);
  }

  // Get a list of elements that are currently invalid (before any negations are applied)
  // This cannot be quick, as we need a full list (for the purposes of negation).
  auto regionData = RegionCheck(puzzle, region, false);
  console.debug("Negation-less regioncheck valid:", regionData.Valid());

  // Set "nonce" back to "nega" for the negation symbols
  for (Cell* cell : negationSymbols) {
    cell->type = "nega";
  }

  auto invalidElements = regionData.invalidElements;
  auto veryInvalidElements = regionData.veryInvalidElements;

  /* Shouldn"t need to do this, since I'm using Cell* instead of grid references. Hmm.
  for (int i=0; i<invalidElements.size(); i++) {
    invalidElements[i] = &puzzle._grid[invalidElements[i]->x][invalidElements[i]->y];
  }
  for (int i=0; i<veryInvalidElements.size(); i++) {
    veryInvalidElements[i] = &puzzle._grid[veryInvalidElements[i]->x][veryInvalidElements[i]->y];
  }
  */

  console.debug("Forcibly negating", veryInvalidElements.size(), "symbols");

  vector<tuple<Cell*, Cell*, string, string>> baseCombination;
  while (negationSymbols.size() > 0 && veryInvalidElements.size() > 0) {
    Cell* source = Pop(negationSymbols);
    Cell* target = Pop(veryInvalidElements);
    baseCombination.emplace_back(source, target, source->type, target->type);
    source->type.clear();
    target->type.clear();
  }

  regionData = RegionCheckNegations2(puzzle, region, negationSymbols, invalidElements);

  // Restore required negations
  for (const auto& [source, target, sourceType, targetType] : baseCombination) {
    source->type = sourceType;
    target->type = targetType;
    regionData.negations.emplace_back(source, target);
  }

  return regionData;
}

RegionData Validator::RegionCheck(const Puzzle& puzzle, const Region& region, bool quick) {
  console.log("Validating region of size", region.cells.size(), region);
  RegionData regionData;

  vector<Cell*> squares;
  vector<Cell*> stars;
  unordered_map<string, int> coloredObjects;
  string squareColor;

  for (auto& [x, y] : region.cells) {
    Cell* cell = &puzzle._grid[x][y];
    if (cell->type.empty()) continue;

    // Check for uncovered dots
    if (cell->dot > DOT_NONE) {
      console.log("Dot at", x, y, "is not covered");
      regionData.veryInvalidElements.push_back(cell);
      if (quick) return regionData;
    }

    // Check for triangles
    if (cell->type == "triangle") {
      int count = 0;
      if (puzzle.GetLine(x - 1, y) > LINE_NONE) count++;
      if (puzzle.GetLine(x + 1, y) > LINE_NONE) count++;
      if (puzzle.GetLine(x, y - 1) > LINE_NONE) count++;
      if (puzzle.GetLine(x, y + 1) > LINE_NONE) count++;
      if (cell->count != count) {
        console.log("Triangle at grid[" + to_string(x) + "][" + to_string(y) + "] has", count, "borders");
        regionData.veryInvalidElements.push_back(cell);
        if (quick) return regionData;
      }
    }

    // Count color-based elements
    if (!cell->color.empty()) {
      int count = GetValueOrDefault(coloredObjects, cell->color, 0);
      coloredObjects[cell->color] = count + 1;

      if (cell->type == "square") {
        squares.push_back(cell);
        if (squareColor.empty()) {
          squareColor = cell->color;
        } else if (squareColor != cell->color) {
          squareColor = "collision"; // Signal value which indicates square color collision
        }
      }

      if (cell->type == "star") {
        stars.push_back(cell);
      }
    }
  }

  if (squareColor == "collision") {
    Append(regionData.invalidElements, squares);
    if (quick) return regionData;
  }

  for (Cell* star : stars) {
    int count = GetValueOrDefault(coloredObjects, star->color, 0);
    if (count == 1) {
      console.log("Found a", star->color, "star in a region with 1", star->color, "object");
      regionData.veryInvalidElements.push_back(star);
      if (quick) return regionData;
    } else if (count > 2) {
      console.log("Found a", star->color, "star in a region with", count, star->color, "objects");
      regionData.invalidElements.push_back(star);
      if (quick) return regionData;
    }
  }

  if (puzzle._hasPolyominos) {
    if (!Polyominos::PolyFit(region, puzzle)) {
      for (const auto [x, y] : region.cells) {
        Cell* cell = puzzle.GetCell(x, y);
        if (cell == nullptr) continue;
        if (cell->type == "poly" || cell->type == "ylop") {
          regionData.invalidElements.push_back(cell);
          if (quick) return regionData;
        }
      }
    }
  }

  console.debug("Region has", regionData.veryInvalidElements.size(), "very invalid elements");
  console.debug("Region has", regionData.invalidElements.size(), "invalid elements");
  return regionData;
}
