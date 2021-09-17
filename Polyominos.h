#pragma once
#include <vector>

struct Cell;
struct Region;
class Puzzle;

class Polyominos {
public:
  // Attempt to fit polyominos in a region into the puzzle.
  // This function checks for early exits, then simplifies the grid to a numerical representation:
  // * 1 represents a square that has been double-covered (by two polyominos)
  //   * Or, in the cancellation case, it represents a square that was covered by a polyomino and not by an onimoylop
  // * 0 represents a square that is satisfied, either because:
  //   * it is outside the region
  //   * (In the normal case) it was inside the region, and has been covered by a polyomino
  //   * (In the cancellation case) it was covered by an equal number of polyominos and onimoylops
  // * -1 represents a square that needs to be covered once (inside the region, or outside but covered by an onimoylop)
  // * -2 represents a square that needs to be covered twice (inside the region & covered by an onimoylop)
  // * And etc, for additional layers of polyominos/onimoylops.
  static bool PolyFit(const Region& region, Puzzle& puzzle);

private:
  static int GetPolySize(int polyshape);
  // IMPORTANT NOTE: When formulating these, the top row must contain (0, 0)
  // That means there will never be any negative y values.
  // (0, 0) must also be a cell in the shape, so that
  // placing the shape at (x, y) will fill (x, y)
  // Ylops will have -1s on all adjacent cells, to break "overlaps" for polyominos.
  static std::vector<std::tuple<int, int>> PolyominoFromPolyshape(unsigned short polyshape, bool ylop=false, bool precise=false);
  // If false, poly doesn"t fit and grid is unmodified
  // If true, poly fits and grid is modified (with the placement)
  static bool TryPlacePolyshape(const std::vector<std::tuple<int, int>>& cells, int x, int y, Puzzle& puzzle, int** polyGrid, int sign);
  // Places the ylops such that they are inside of the grid, then checks if the polys
  // zero the region.
  static bool PlaceYlops(const std::vector<Cell*>& ylops, int i, std::vector<Cell*>& polys, Puzzle& puzzle, int** polyGrid);
  // Returns whether or not a set of polyominos fit into a region.
  // Solves via recursive backtracking: Some piece must fill the top left square,
  // so try every piece to fill it, then recurse.
  static bool PlacePolys(std::vector<Cell*>& polys, Puzzle& puzzle, int** polyGrid);

  static inline int Mask(int x, int y) {
    return 1 << (x * 4 + y);
  }

  static inline bool IsSet(int polyshape, int x, int y) {
    if (x < 0 || y < 0) return false;
    if (x >= 4 || y >= 4) return false;
    return (polyshape & Mask(x, y)) != 0;
  }
  static inline std::vector<unsigned short> GetRotations(unsigned short polyshape) {
    return { polyshape }; // When we implement this for real, delete Random::RotatePolyshape.
  }

};