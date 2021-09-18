#include <unordered_set>

#include "Polyominos.h"
#include "Puzzle.h"
#include "Utilities.h"

using namespace std;

int Polyominos::GetPolySize(int polyshape) {
  int size = 0;
  for (int x=0; x<4; x++) {
    for (int y=0; y<4; y++) {
      if (IsSet(polyshape, x, y)) size++;
    }
  }
  return size;
}

vector<tuple<int, int>> Polyominos::PolyominoFromPolyshape(unsigned short polyshape, bool ylop, bool precise) {
  int topLeftX = -1;
  int topLeftY = -1;
  for (int x=0; x<4; x++) {
    for (int y=0; y<4; y++) {
      if (IsSet(polyshape, x, y)) {
        topLeftX = x;
        topLeftY = y;
        break;
      }
    }
    if (topLeftX != -1) break;
  }
  if (topLeftX == -1) return {}; // Empty polyomino

  vector<tuple<int, int>> polyomino;
  for (int x=0; x<4; x++) {
    for (int y=0; y<4; y++) {
      if (!IsSet(polyshape, x, y)) continue;
      polyomino.emplace_back(2*(x - topLeftX), 2*(y - topLeftY));

      // "Precise" polyominos adds cells in between the apparent squares in the polyomino.
      // This prevents the solution line from going through polyominos in the solution.
      if (precise) {
        if (ylop) {
          // Ylops fill up/left if no adjacent cell, and always fill bottom/right
          if (!IsSet(polyshape, x - 1, y)) {
            polyomino.emplace_back(2*(x - topLeftX) - 1, 2*(y - topLeftY));
          }
          if (!IsSet(polyshape, x, y - 1)) {
            polyomino.emplace_back(2*(x - topLeftX), 2*(y - topLeftY) - 1);
          }
          polyomino.emplace_back(2*(x - topLeftX) + 1, 2*(y - topLeftY));
          polyomino.emplace_back(2*(x - topLeftX), 2*(y - topLeftY) + 1);
        } else {
          // Normal polys only fill bottom/right if there is an adjacent cell.
          if (IsSet(polyshape, x + 1, y)) {
            polyomino.emplace_back(2*(x - topLeftX) + 1, 2*(y - topLeftY));
          }
          if (IsSet(polyshape, x, y + 1)) {
            polyomino.emplace_back(2*(x - topLeftX), 2*(y - topLeftY) + 1);
          }
        }
      }
    }
  }
  return polyomino;
}

bool Polyominos::PolyFit(const Region& region, const Puzzle& puzzle) {
  vector<Cell*> polys;
  vector<Cell*> ylops;
  int polyCount = 0;
  int regionSize = 0;
  for (auto [x, y] : region.cells) {
    if (x%2 == 1 && y%2 == 1) regionSize++;
    Cell* cell = &puzzle._grid[x][y];
    if (cell->polyshape == 0) continue;
    if (cell->TypeIs("poly")) {
      polys.push_back(cell);
      polyCount += GetPolySize(cell->polyshape);
    } else if (cell->TypeIs("ylop")) {
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
    if (true /* puzzle.settings.SHAPELESS_ZERO_POLY */) {
      console.log("Combined size of polyominos and onimoylops is zero");
      return true;
    }
  }

  // For polyominos, we clear the grid to mark it up again:
  // First, we mark all cells as 0: Cells outside the target region should be unaffected.
  int** polyGrid = new int*[puzzle._width];
  for (int x=0; x<puzzle._width; x++) {
    polyGrid[x] = new int[puzzle._height];
    memset(polyGrid[x], 0, puzzle._height);
  }

  // In the normal case, we mark every cell as -1: It needs to be covered by one poly
  if (polyCount > 0) {
    for (auto [x, y] : region.cells) polyGrid[x][y] = -1;
  }
  // In the exact match case, we leave every cell marked 0: Polys and ylops need to cancel.

  bool ret = PlaceYlops(ylops, 0, polys, puzzle, polyGrid);

  for (int x=0; x<puzzle._width; x++) delete polyGrid[x];
  delete[] polyGrid;

  return ret;
}

bool Polyominos::TryPlacePolyshape(const vector<tuple<int, int>>& cells, int x, int y, const Puzzle& puzzle, int** polyGrid, int sign) {
  console.spam("Placing at", x, y, "with sign", sign);
  vector<int> values(cells.size(), 0);
  for (int i=0; i<cells.size(); i++) {
    auto [cellX, cellY] = cells[i];
    if (!puzzle._safeCell(puzzle._mod(cellX + x), cellY + y)) return false; // Hackity hack.
    int puzzleCell = polyGrid[cellX + x][cellY + y];
    values[i] = puzzleCell;
  }
  for (int i=0; i<cells.size(); i++) {
    auto [cellX, cellY] = cells[i];
    polyGrid[cellX + x][cellY + y] = values[i] + sign;
  }
  return true;
}

bool Polyominos::PlaceYlops(const vector<Cell*>& ylops, int i, vector<Cell*>& polys, const Puzzle& puzzle, int** polyGrid) {
  // Base case: No more ylops to place, start placing polys
  if (i == ylops.size()) return PlacePolys(polys, puzzle, polyGrid);

  *static_cast<volatile int*>(nullptr) = 1; // assert(false)
  return false;
}

bool Polyominos::PlacePolys(std::vector<Cell*>& polys, const Puzzle& puzzle, int** polyGrid) {
  // Check for overlapping polyominos, and handle exit cases for all polyominos placed.
  bool allPolysPlaced = (polys.size() == 0);
  for (int x=0; x<puzzle._width; x++) {
    for (int y=0; y<puzzle._height; y++) {
      int cell = polyGrid[x][y];
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
  vector<tuple<int, int>> openCells;
  for (int y=1; y<puzzle._height; y+=2) {
    for (int x=1; x<puzzle._width; x+=2) {
      if (polyGrid[x][y] >= 0) continue;
      openCells.emplace_back(x, y);
      if (puzzle._pillar == false) break;
    }
    if (openCells.size() > 0) break;
  }

  if (openCells.size() == 0) {
    console.log("Polys remaining but grid full");
    return false;
  }

  for (auto [openCellX, openCellY] : openCells) {
    unordered_set<unsigned short> attemptedPolyshapes;
    for (int i=0; i<polys.size(); i++) {
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
        vector<tuple<int, int>> cells = PolyominoFromPolyshape(polyshape, false, false /*puzzle.settings.PRECISE_POLYOMINOS*/);
        if (!TryPlacePolyshape(cells, openCellX, openCellY, puzzle, polyGrid, +1)) {
          console.spam("Polyshape", polyshape, "does not fit into", openCellX, openCellY);
          continue;
        }
        // console.group("");
        if (PlacePolys(polys, puzzle, polyGrid)) return true;
        // console.groupEnd("")
        // Should not fail, as it"s an inversion of the above tryPlacePolyshape
        TryPlacePolyshape(cells, openCellX, openCellY, puzzle, polyGrid, -1);
      }
      polys.insert(polys.begin() + i, poly); // polys.splice(i, 0, poly)
    }
  }
  console.log("Grid non-empty with >0 polys, but no valid recursion.");
  return false;
}



