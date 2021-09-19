#include "stdafx.h"

#include <unordered_map>

RegionData Validator::Validate(Puzzle& puzzle, bool quick) {
  // console.log("Validating", puzzle._name);
  RegionData puzzleData;

  bool needsRegions = false;
  Region monoRegion(puzzle._width * puzzle._height);
  // These two are both used by validateRegion, so they are saved on the puzzle itself.
  puzzle._hasNegations = false;
  puzzle._hasPolyominos = false;

  // Validate gap failures as an early exit.
  for (int x=0; x<puzzle._width; x++) {
    for (int y=0; y<puzzle._height; y++) {
      Cell* cell = &puzzle._grid[x][y]; // Copy is probably cheaper here, since we reference some data more than once.
      // if (cell == nullptr) continue;
      if (!needsRegions && cell->type != CELL_TYPE_LINE && cell->type != CELL_TYPE_TRIANGLE) needsRegions = true;
      if (cell->type == CELL_TYPE_NEGA) puzzle._hasNegations = true;
      if (cell->type == CELL_TYPE_POLY || cell->type == CELL_TYPE_YLOP) puzzle._hasPolyominos = true;
      if (cell->line > Line::None) {
        if (cell->gap > GAP_NONE) {
          console.log("Solution line goes over a gap at", x, y);
          puzzleData.veryInvalidElements.push_back(cell);
          if (quick) return puzzleData;
        }
        if ((cell->dot == Dot::Blue && cell->line == Line::Yellow) ||
            (cell->dot == Dot::Yellow && cell->line == Line::Blue)) {
          console.log("Incorrectly covered dot: Dot is", (u8)cell->dot, "but line is", (u8)cell->line);
          puzzleData.veryInvalidElements.push_back(cell);
          if (quick) return puzzleData;
        }
      } else if (!needsRegions) { // We can stop building the monoRegion if we actually need regions.
        monoRegion.EmplaceBack({ (u8)x, (u8)y });
      }
    }
  }

  Vector<Region> regions(5);
  if (needsRegions) {
    regions = puzzle.GetRegions();
  } else {
    regions.EmplaceBack(move(monoRegion));
  }
  console.log("Found", regions.Size(), "region(s)");
  // console.debug(regions);

  for (const Region& region : regions) {
    auto regionData = ValidateRegion(puzzle, region, quick);
    console.log("Region valid:", regionData.Valid());
    Append(puzzleData.negations, regionData.negations);
    Append(puzzleData.invalidElements, regionData.invalidElements);
    Append(puzzleData.veryInvalidElements, regionData.veryInvalidElements);
    if (quick && !puzzleData.Valid()) break;
  }
  console.log("Puzzle has", puzzleData.invalidElements.size(), "invalid elements");
  return puzzleData;
}

RegionData Validator::ValidateRegion(const Puzzle& puzzle, const Region& region, bool quick) {
  if (!puzzle._hasNegations) return RegionCheck(puzzle, region, quick);

  // Get a list of negation symbols in the grid, and set them to "nonce"
  vector<Cell*> negationSymbols;
  for (const auto [x, y] : region) {
    Cell* cell = &puzzle._grid[x][y];
    if (cell->type == CELL_TYPE_NEGA) {
      cell->type = CELL_TYPE_NONCE;
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
    cell->type = CELL_TYPE_NEGA;
  }

  auto invalidElements = regionData.invalidElements;
  auto veryInvalidElements = regionData.veryInvalidElements;
  // We don't need to repopulate these, since we're using cell references.
  console.debug("Forcibly negating", veryInvalidElements.size(), "symbols");

  vector<tuple<Cell*, Cell*, u8, u8>> baseCombination;
  while (negationSymbols.size() > 0 && veryInvalidElements.size() > 0) {
    Cell* source = Pop(negationSymbols);
    Cell* target = Pop(veryInvalidElements);
    baseCombination.emplace_back(source, target, source->type, target->type);
    source->type = CELL_TYPE_NULL;
    target->type = CELL_TYPE_NULL;
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

RegionData Validator::RegionCheckNegations2(
  const Puzzle& puzzle,
  const Region& region,
  const vector<Cell*>& negationSymbols,
  const vector<Cell*>& invalidElements,
  int index,
  int index2) {
  (void)puzzle;
  (void)region;
  (void)negationSymbols;
  (void)invalidElements;
  (void)index;
  (void)index2;

  return RegionData();
}

using ColoredObjectArr = vector<pair<int, u8>>;
u8 GetColoredObject(const ColoredObjectArr& coloredObjects, int color_) {
  for (auto [color, count] : coloredObjects) {
    if (color == color_) return count;
  }
  return 0;
}

void AddColoredObject(ColoredObjectArr& coloredObjects, int color_) {
  for (auto& it : coloredObjects) {
    if (it.first == color_) {
      it.second++;
      return;
    }
  }
  coloredObjects.emplace_back(color_, (u8)1);
}

RegionData Validator::RegionCheck(const Puzzle& puzzle, const Region& region, bool quick) {
  // console.log("Validating region of size", region.size());
  RegionData regionData;

  vector<Cell*> squares;
  squares.reserve(4);
  vector<Cell*> stars;
  stars.reserve(4);
  ColoredObjectArr coloredObjects;
  coloredObjects.reserve(4);
  int squareColor = 0;

  for (auto& [x, y] : region) {
    Cell* cell = &puzzle._grid[x][y];
    switch (cell->type) {
      case CELL_TYPE_NULL:
      default:
        continue;

      case CELL_TYPE_LINE:
        // Check for uncovered dots
        if (cell->dot > Dot::None) {
          console.log("Dot at", x, y, "is not covered");
          regionData.veryInvalidElements.push_back(cell);
          if (quick) return regionData;
        }
        continue;

      case CELL_TYPE_TRIANGLE:
        {
          int count = 0;
          if (puzzle.GetLine(x - 1, y) > Line::None) count++;
          if (puzzle.GetLine(x + 1, y) > Line::None) count++;
          if (puzzle.GetLine(x, y - 1) > Line::None) count++;
          if (puzzle.GetLine(x, y + 1) > Line::None) count++;
          if (cell->count != count) {
            console.log("Triangle at grid[" + to_string(x) + "][" + to_string(y) + "] has", count, "borders");
            regionData.veryInvalidElements.push_back(cell);
            if (quick) return regionData;
          }
        }
        continue;

      case CELL_TYPE_SQUARE:
        squares.push_back(cell);
        AddColoredObject(coloredObjects, cell->color);
        if (squareColor == 0) {
          squareColor = cell->color;
        } else if (squareColor != cell->color) {
          squareColor = -1; // Signal value which indicates square color collision
        }
        continue;

      case CELL_TYPE_STAR:
        stars.push_back(cell);
        AddColoredObject(coloredObjects, cell->color);
        continue;
    }
  }

  if (squareColor == -1) {
    for (Cell* square : squares) {
      regionData.invalidElements.push_back(square);
      if (quick) return regionData;
    }
  }

  for (Cell* star : stars) {
    int count = GetColoredObject(coloredObjects, star->color);
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
      for (const auto [x, y] : region) {
        Cell* cell = puzzle.GetCell(x, y);
        if (cell == nullptr) continue;
        if (cell->type == CELL_TYPE_POLY || cell->type == CELL_TYPE_YLOP) {
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
