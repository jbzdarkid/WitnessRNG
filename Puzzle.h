#pragma once
#include <string>
#include <vector>
struct Random;

struct Cell {
  std::string type;
  int x = 0;
  int y = 0;
  int dot = 0;
  int gap = 0;
  int line = 0;
  int count = 0;
  unsigned short polyshape = 0u;
  bool start = false;

  std::string end;
  std::string color;

  std::string ToString(int x, int y);
};

struct Region {
  std::vector<std::tuple<int, int>> cells;
  std::vector<int> grid;

  Region(int length) {
    grid = std::vector<int>(length, 0);
  }

  inline bool GetCell(int x, int y) {
    return ((grid[x] & (1 << y)) != 0);
  }

  inline void SetCell(int x, int y) {
    if (GetCell(x, y)) return;
    grid[x] |= (1 << y);
    cells.emplace_back(x, y);
  }
};

class Puzzle {
public:
  int _origWidth = 0;
  int _origHeight = 0;
  int _height = 0;
  int _width = 0;
  bool _pillar = false;
  int _numConnections = 0;
  std::string _name;
  std::vector<std::tuple<int, int>> _connections;
  Cell** _grid;

  // Properties set and read by Validate()
  bool _valid = false;
  bool _hasNegations = false;
  bool _hasPolyominos = false;
  std::vector<std::tuple<Cell*, Cell*>> _negations;
  std::vector<Cell*> _invalidElements;
  std::vector<Cell*> _veryInvalidElements;

  // Non-RNG functions from WP
  Puzzle(int width, int height, bool pillar=false);

  void SetCell(int x, int y, Cell cell);
  Cell* GetCell(int x, int y);
  // A variant of getCell which specifically returns line values,
  // and treats objects as being out-of-bounds
  int GetLine(int x, int y);
  std::vector<Region> GetRegions();

  // RNG functions (from TW)
  static Puzzle GeneratePolyominos(Random& rng);

  void CutRandomEdges(Random& rng, int numCuts);
  void AddRandomDots(Random& rng, int numDots);
  std::tuple<int, int> GetEmptyCell(Random& rng);

private:
  int _mod(int x);
  bool _safeCell(int x, int y);
};

std::ostream& operator<<(std::ostream& os, const Puzzle& p);
std::ostream& operator<<(std::ostream& os, const Region& r);
