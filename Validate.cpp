#include "stdafx.h"

RegionData Validator::Validate(Puzzle& puzzle, bool quick) {
  console.log("Validating", puzzle._name);
  RegionData puzzleData(quick ? 0 : puzzle._width * puzzle._height);

  bool needsRegions = false;
  int monoRegionSize = 0;
  // These two are both used by validateRegion, so they are saved on the puzzle itself.
  puzzle._hasNegations = false;
  puzzle._hasPolyominos = false;

  // Validate gap failures as an early exit.
  for (u8 x=0; x<puzzle._width; x++) {
    for (u8 y=0; y<puzzle._height; y++) {
      Cell* cell = &puzzle._grid->Get(x, y);
      switch (cell->type) {
        case Type::Nega:
          puzzle._hasNegations = true;
          break;
        case Type::Poly:
        case Type::Ylop:
          puzzle._hasPolyominos = true;
          break;
        case Type::Null:
        case Type::Triangle:
          break;
        default:
          needsRegions = true;
          break;
        case Type::Line:
          if (cell->line == Line::None) {
            monoRegionSize++;
          } else {
            if (cell->gap != Gap::None) {
              console.log("Solution line goes over a gap at", x, y);
              puzzleData.veryInvalidElements.Push(cell);
              if (quick) return puzzleData;
            }
            if ((cell->dot == Dot::Blue && cell->line == Line::Yellow) ||
                (cell->dot == Dot::Yellow && cell->line == Line::Blue)) {
              console.log("Incorrectly covered dot: Dot is", (u8)cell->dot, "but line is", (u8)cell->line);
              puzzleData.veryInvalidElements.Push(cell);
              if (quick) return puzzleData;
            }
          }
          break;
      }
    }
  }

  Vector<Region> regions;
  if (needsRegions) {
    regions = puzzle.GetRegions();
  } else {
    Region monoRegion(monoRegionSize);
    for (u8 x=0; x<puzzle._width; x++) {
      for (u8 y=0; y<puzzle._height; y++) {
        Cell* cell = &puzzle._grid->Get(x, y);
        if (cell->type == Type::Line && cell->line == Line::None) monoRegion.UnsafePush(cell);
      }
    }
    regions.Emplace(move(monoRegion));
  }
  console.log("Found", regions.Size(), "region(s)");

  for (const Region& region : regions) {
    auto regionData = ValidateRegion(puzzle, region, quick);
    console.log("Region valid:", regionData.Valid());
    puzzleData.negations.Append(regionData.negations);
    puzzleData.invalidElements.Append(regionData.invalidElements);
    puzzleData.veryInvalidElements.Append(regionData.veryInvalidElements);
    if (quick && !puzzleData.Valid()) break;
  }
  console.log("Puzzle has", puzzleData.invalidElements.Size(), "invalid elements");
  return puzzleData;
}

RegionData Validator::ValidateRegion(const Puzzle& puzzle, const Region& region, bool quick) {
  if (!puzzle._hasNegations) return RegionCheck(puzzle, region, quick);

  // Get a list of negation symbols in the grid, and set them to "nonce"
  Vector<Cell*> negationSymbols(puzzle._width);
  for (Cell* cell : region) {
    if (cell->type == Type::Nega) {
      cell->type = Type::Nonce;
      negationSymbols.Push(cell);
    }
  }
  console.debug("Found", negationSymbols.Size(), "negation symbols");
  if (negationSymbols.Empty()) {
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
    cell->type = Type::Nega;
  }

  auto& invalidElements = regionData.invalidElements;
  auto& veryInvalidElements = regionData.veryInvalidElements;
  // We don't need to repopulate these, since we're using cell references.
  console.debug("Forcibly negating", veryInvalidElements.Size(), "symbols");

  Vector<tuple<Cell*, Cell*, Type, Type>> baseCombination;
  while (!negationSymbols.Empty() && !veryInvalidElements.Empty()) {
    Cell* source = negationSymbols.PopValue();
    Cell* target = veryInvalidElements.PopValue();
    baseCombination.Emplace({source, target, source->type, target->type});
    source->type = Type::Null;
    target->type = Type::Null;
  }

  regionData = RegionCheckNegations2(puzzle, region, negationSymbols, invalidElements);

  // Restore required negations
  for (const auto& [source, target, sourceType, targetType] : baseCombination) {
    source->type = sourceType;
    target->type = targetType;
    regionData.negations.Emplace({ source, target });
  }

  return regionData;
}

RegionData Validator::RegionCheckNegations2(
  const Puzzle& puzzle,
  const Region& region,
  const Vector<Cell*>& negationSymbols,
  const Vector<Cell*>& invalidElements,
  u8 index,
  u8 index2) {
  (void)puzzle;
  (void)region;
  (void)negationSymbols;
  (void)invalidElements;
  (void)index;
  (void)index2;

  return RegionData(0);
}

using ColoredObjectArr = Vector<pair<int, u8>>;
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
  coloredObjects.Emplace({ color_, (u8)1 });
}

RegionData Validator::RegionCheck(const Puzzle& puzzle, const Region& region, bool quick) {
  console.log("Validating region of size", region.Size());
  RegionData regionData(quick ? 0 : region.Size());

  // We could re-use these! Just make a Validator class.
  Vector<Cell*> squares;
  Vector<Cell*> stars;
  ColoredObjectArr coloredObjects;
  int squareColor = 0;

  for (Cell* cell : region) {
    switch (cell->type) {
      case Type::Null:
      default:
        continue;

      case Type::Line:
        // Check for uncovered dots
        if (cell->dot != Dot::None) {
          console.log("Dot at", cell->x, cell->y, "is not covered");
          regionData.veryInvalidElements.Push(cell);
          if (quick) return regionData;
        }
        continue;

      case Type::Triangle:
        {
          u8 count = 0;
          if (puzzle.GetLine(cell->x - 1, cell->y) != Line::None) count++;
          if (puzzle.GetLine(cell->x + 1, cell->y) != Line::None) count++;
          if (puzzle.GetLine(cell->x, cell->y - 1) != Line::None) count++;
          if (puzzle.GetLine(cell->x, cell->y + 1) != Line::None) count++;
          if (cell->count != count) {
            console.log("Cell ", cell, "has", count, "borders");
            regionData.veryInvalidElements.Push(cell);
            if (quick) return regionData;
          }
        }
        continue;

      case Type::Square:
        squares.Push(cell);
        AddColoredObject(coloredObjects, cell->color);
        if (squareColor == 0) {
          squareColor = cell->color;
        } else if (squareColor != cell->color) {
          squareColor = -1; // Signal value which indicates square color collision
        }
        continue;

      case Type::Star:
        stars.Push(cell);
        AddColoredObject(coloredObjects, cell->color);
        continue;
    }
  }

  if (squareColor == -1) {
    for (Cell* square : squares) {
      regionData.invalidElements.Push(square);
      if (quick) return regionData;
    }
  }

  for (Cell* star : stars) {
    u8 count = GetColoredObject(coloredObjects, star->color);
    if (count == 1) {
      console.log("Found a", star->color, "star in a region with 1", star->color, "object");
      regionData.veryInvalidElements.Push(star);
      if (quick) return regionData;
    } else if (count > 2) {
      console.log("Found a", star->color, "star in a region with", count, star->color, "objects");
      regionData.invalidElements.Push(star);
      if (quick) return regionData;
    }
  }

  if (puzzle._hasPolyominos) {
    if (!Polyominos::PolyFit(region, puzzle)) {
      for (Cell* cell : region) {
        if (cell->type == Type::Poly || cell->type == Type::Ylop) {
          regionData.invalidElements.Push(cell);
          if (quick) return regionData;
        }
      }
    }
  }

  console.debug("Region has", regionData.veryInvalidElements.Size(), "very invalid elements");
  console.debug("Region has", regionData.invalidElements.Size(), "invalid elements");
  return regionData;
}
