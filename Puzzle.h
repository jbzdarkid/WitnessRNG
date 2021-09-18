#pragma once
#include <string>
#include <vector>
#include "Utilities.h"
class Random;

using u8 = unsigned char;

struct Cell {
  u8 type;
  u8 x = 0;
  u8 y = 0;

  Dot dot = Dot::None;
  u8 gap = 0;
  u8 count = 0;
  Line line = Line::None;
  unsigned short polyshape = 0u;
  int color;

  u8 end;
  bool start = false;

  std::string ToString(int x, int y);
};

struct Region {
public:
  std::vector<std::tuple<int, int>> cells;

  Region(int length) {
    _grid = new int[length]; // std::vector<int>(length, 0);
    memset(_grid, 0, length);
  }

  ~Region() {
    delete[] _grid;
  }

  DELETE_RO3(Region)
  // RO5
  Region(Region&& other) noexcept {
    _grid = other._grid;
    other._grid = nullptr;
  }
  Region& operator=(Region&& other) noexcept {
    _grid = other._grid;
    other._grid = nullptr;
    return *this;
  }

  bool GetCell(int x, int y) {
    return ((_grid[x] & (1 << y)) != 0);
  }

  void SetCell(int x, int y) {
    if (GetCell(x, y)) return;
    _grid[x] |= (1 << y);
    cells.emplace_back(x, y);
  }

private:
  int* _grid;
};

class Puzzle {
public:
  int _origWidth = 0;
  int _origHeight = 0;
  int _height = 0;
  int _width = 0;
  bool _pillar = false;
  int _numConnections = 0;
  int _symmetry = 0;
  std::string _name;
  std::vector<std::tuple<int, int>> _connections;
  Cell** _grid;
  int** _maskedGrid;

  // Properties set and read by Validate() / Solve()
  bool _valid = false;
  bool _hasNegations = false;
  bool _hasPolyominos = false;
  Cell* _startPoint = nullptr;
  Cell* _endPoint = nullptr;
  std::vector<std::tuple<Cell*, Cell*>> _negations;
  std::vector<Cell*> _invalidElements;
  std::vector<Cell*> _veryInvalidElements;

  // Non-RNG functions from WP... ish
  Puzzle(int width, int height, bool pillar=false);
  ~Puzzle();
  DELETE_RO3(Puzzle)
  DELETE_RO5(Puzzle)

  // void SetCell(int x, int y, Cell cell);
  Cell* GetCell(int x, int y) const;
  Cell* GetSymmetricalCell(Cell* cell) { return nullptr; }
  bool MatchesSymmetricalPos(int x, int y, int symX, int symY) { return false; }
  // A variant of getCell which specifically returns line values,
  // and treats objects as being out-of-bounds
  Line GetLine(int x, int y) const;
  void ClearGrid();

  void _floodFill(int x, int y, Region& region);
  void GenerateMaskedGrid();
  std::vector<Region> GetRegions();
  Region GetRegion(int x, int y);

  // RNG functions (from TW)
  void CutRandomEdges(Random& rng, int numCuts);
  // void AddRandomDots(Random& rng, int numDots);
  Cell* GetEmptyCell(Random& rng);

// private:
  int _mod(int x) const;
  bool _safeCell(int x, int y) const;
};

std::ostream& operator<<(std::ostream& os, const Puzzle& p);
std::ostream& operator<<(std::ostream& os, const Region& r);
