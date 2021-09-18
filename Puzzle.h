#pragma once
#include <string>
#include <vector>
class Random;

#define RO3(clazz) \
  clazz##(const clazz & other) = delete; /* Copy constructor */ \
  clazz & operator=(const clazz & other) = delete; /* Copy assignment */

#define RO5(clazz) \
  RO3(clazz) \

using u8 = unsigned char;

#define TYPELESS "\0"
struct Cell {
  char type[10]; // triangle + 1
  int x = 0;
  int y = 0;

  u8 dot = 0;
  u8 gap = 0;
  u8 line = 0;
  u8 count = 0;
  unsigned short polyshape = 0u;
  bool start = false;

  u8 end;
  int color;

  std::string ToString(int x, int y);
  inline bool TypeIs(const char* type_) const {
    return strncmp(type, type_, sizeof(type)) == 0;
  }
  inline void SetType(const char* type_) {
    strncpy_s(type, type_, sizeof(type));
  }
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

  RO3(Region)
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
  RO3(Puzzle);
  // RO5
  Puzzle(Puzzle&& other) noexcept = delete;
  Puzzle& operator=(Puzzle&& other) noexcept = delete;\
  /*
  Puzzle(Puzzle&& other) noexcept {
    _grid = other._grid;
    other._grid = nullptr;
  }
  Puzzle& operator=(Puzzle&& other) noexcept {
    _grid = other._grid;
    other._grid = nullptr;
    return *this;
  }
  */

  // void SetCell(int x, int y, Cell cell);
  Cell* GetCell(int x, int y) const;
  Cell* GetSymmetricalCell(Cell* cell) { return nullptr; }
  bool MatchesSymmetricalPos(int x, int y, int symX, int symY) { return false; }
  // A variant of getCell which specifically returns line values,
  // and treats objects as being out-of-bounds
  int GetLine(int x, int y) const;
  void ClearGrid();

  void _floodFill(int x, int y, Region& region, int** maskedGrid);
  int** GenerateMaskedGrid();
  std::vector<Region> GetRegions();
  Region GetRegion(int x, int y);

  // RNG functions (from TW)
  void CutRandomEdges(Random& rng, int numCuts);
  void AddRandomDots(Random& rng, int numDots);
  Cell* GetEmptyCell(Random& rng);

// private:
  int _mod(int x) const;
  bool _safeCell(int x, int y) const;
};

std::ostream& operator<<(std::ostream& os, const Puzzle& p);
std::ostream& operator<<(std::ostream& os, const Region& r);
