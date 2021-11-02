#include "stdafx.h"
#include <unordered_set>
#include <utility> // For pair

u8 Polyominos::GetPolySize(u16 polyshape) {
  u8 size = 0;
  for (u8 x=0; x<4; x++) {
    for (u8 y=0; y<4; y++) {
      if (IsSet(polyshape, x, y)) size++;
    }
  }
  return size;
}

Polyomino Polyominos::PolyominoFromPolyshape(u16 polyshape) {
  s8 topLeftX = -1;
  s8 topLeftY = -1;
  for (u8 x=0; x<4; x++) {
    for (u8 y=0; y<4; y++) {
      if (IsSet(polyshape, x, y)) {
        topLeftX = x;
        topLeftY = y;
        break;
      }
    }
    if (topLeftX != -1) break;
  }
  if (topLeftX == -1) return {}; // Empty polyomino

  Polyomino polyomino;
  for (u8 x=0; x<4; x++) {
    for (u8 y=0; y<4; y++) {
      if (!IsSet(polyshape, x, y)) continue;
      polyomino.emplace_back(make_pair<s8, s8>(2*(x - topLeftX), 2*(y - topLeftY)));
    }
  }
  return polyomino;
}

bool Polyominos::PolyFit(const Region& region, const Puzzle& puzzle) {
  vector<Cell*> polys;
  vector<Cell*> ylops;
  s8 polyCount = 0;
  u8 regionSize = 0;
  for (Cell* cell : region) {
    if (cell->x%2 == 1 && cell->y%2 == 1) regionSize++;
    if (cell->polyshape == 0) continue;
    if (cell->type == Type::Poly) {
      polys.push_back(cell);
      polyCount += GetPolySize(cell->polyshape);
    } else if (cell->type == Type::Ylop) {
      ylops.push_back(cell);
      polyCount -= GetPolySize(cell->polyshape);
    }
  }
  if (polys.size() + ylops.size() == 0) {
    console.log("No polyominos or onimoylops inside the region, vacuously true");
    return true;
  }
  if (polyCount > 0 && polyCount != regionSize) {
    console.log("Combined size of polyominos and onimoylops", polyCount, "does not match region size", regionSize);
    return false;
  }
  if (polyCount < 0) {
    console.log("Combined size of onimoylops is greater than polyominos by", -polyCount);
    return false;
  }
  if (polyCount == 0) {
    console.log("Combined size of polyominos and onimoylops is zero");
    return true;
  }

  // For polyominos, we clear the grid to mark it up again:
  // First, we mark all cells as 0: Cells outside the target region should be unaffected.
  s8** polyGrid = NewDoubleArray2<s8>(puzzle._width, puzzle._height);

  // In the normal case, we mark every cell as -1: It needs to be covered by one poly
  if (polyCount > 0) {
    for (Cell* cell : region) polyGrid[cell->x][cell->y] = -1;
  }
  // In the exact match case, we leave every cell marked 0: Polys and ylops need to cancel.

  bool ret = PlaceYlops(ylops, 0, polys, puzzle, polyGrid);

  DeleteDoubleArray2(polyGrid);

  return ret;
}

bool Polyominos::TryPlacePolyshape(const Polyomino& cells, u8 x, u8 y, const Puzzle& puzzle, s8** polyGrid, s8 sign) {
  console.spam("Placing at", x, y, "with sign", sign);
  Vector<s8> values((int)cells.size());
  for (u8 i=0; i<cells.size(); i++) {
    auto [cellX, cellY] = cells[i];
    if (puzzle.GetCell(cellX + x, cellY + y) == nullptr) return false;
    s8 puzzleCell = polyGrid[cellX + x][cellY + y];
    values.UnsafePush(puzzleCell);
  }
  for (u8 i=0; i<cells.size(); i++) {
    auto [cellX, cellY] = cells[i];
    polyGrid[cellX + x][cellY + y] = values[i] + sign;
  }
  return true;
}

bool Polyominos::PlaceYlops(const vector<Cell*>& ylops, u8 i, vector<Cell*>& polys, const Puzzle& puzzle, s8** polyGrid) {
  // Base case: No more ylops to place, start placing polys
  if (i == ylops.size()) return PlacePolys(polys, puzzle, polyGrid);

  assert(false);
  return false;
}

bool Polyominos::PlacePolys(std::vector<Cell*>& polys, const Puzzle& puzzle, s8** polyGrid) {
  // Check for overlapping polyominos, and handle exit cases for all polyominos placed.
  bool allPolysPlaced = (polys.size() == 0);
  for (u8 x=0; x<puzzle._width; x++) {
    for (u8 y=0; y<puzzle._height; y++) {
      s8 cell = polyGrid[x][y];
      if (cell > 0) {
        console.log("Cell", x, y, "has been overfilled and no ylops left to place");
        return false;
      }
      if (allPolysPlaced && cell < 0 && x%2 == 1 && y%2 == 1) {
        // Normal, center cell with a negative value & no polys remaining.
        console.log("All polys placed, but grid not full");
        return false;
      }
    }
  }
  if (allPolysPlaced) {
    console.log("All polys placed, and grid full");
    return true;
  }

  // The top-left (first open cell) must be filled by a polyomino.
  // However in the case of pillars, there is no top-left, so we try all open cells in the
  // top-most open row
  Vector<pair<u8, u8>> openCells(puzzle._pillar ? puzzle._width : 0);
  for (u8 y=1; y<puzzle._height; y+=2) {
    for (u8 x=1; x<puzzle._width; x+=2) {
      if (polyGrid[x][y] >= 0) continue;
      openCells.Emplace({x, y});
      if (puzzle._pillar == false) break;
    }
    if (!openCells.Empty()) break;
  }

  if (openCells.Empty()) {
    console.log("Polys remaining but grid full");
    return false;
  }

  for (auto [openCellX, openCellY] : openCells) {
    unordered_set<u16> attemptedPolyshapes;
    for (u8 i=0; i<polys.size(); i++) {
      Cell* poly = polys[i];
      console.spam("Selected poly", poly);
      if (Contains(attemptedPolyshapes, poly->polyshape)) {
        console.spam("Polyshape", poly->polyshape, "has already been attempted");
        continue;
      }
      attemptedPolyshapes.insert(poly->polyshape);
      polys.erase(polys.begin() + i); // polys.splice(i, 1)
      for (auto polyshape : GetRotations(poly->polyshape)) {
        console.spam("Selected polyshape", polyshape);
        Polyomino cells = PolyominoFromPolyshape(polyshape);
        if (!TryPlacePolyshape(cells, openCellX, openCellY, puzzle, polyGrid, +1)) {
          console.spam("Polyshape", polyshape, "does not fit into", openCellX, openCellY);
          continue;
        }
        console.group();
        if (PlacePolys(polys, puzzle, polyGrid)) return true;
        console.groupEnd();
        // Should not fail, as it"s an inversion of the above tryPlacePolyshape
        TryPlacePolyshape(cells, openCellX, openCellY, puzzle, polyGrid, -1);
      }
      polys.insert(polys.begin() + i, poly); // polys.splice(i, 0, poly)
    }
  }
  console.log("Grid non-empty with >0 polys, but no valid recursion.");
  return false;
}

u16 Polyominos::RotatePolyshape(u16 polyshape) {
  u16 newshape = 0;
  for (u8 x=0; x<4; x++) {
    for (u8 y=0; y<4; y++) {
      if (polyshape & (1 << (x*4 + y))) newshape |= 1 << (y*4 + 3-x);
    }
  }
  return newshape;
}

u16 Polyominos::Normalize(u16 polyshape) {
  if (polyshape == 0) { // we would loop forever
    assert(false);
    return 0;
  }
  while ((polyshape & 0x1111) == 0) polyshape >>= 1;
  while ((polyshape & 0x000F) == 0) polyshape >>= 4;
  return polyshape;
}

u16 Polyominos::Flip(u16 polyshape) {
  return (polyshape & 0x000F) << 12 | (polyshape & 0x00F0) << 4 | (polyshape & 0x0F00) >> 4 | (polyshape & 0xF000) >> 12;
}
