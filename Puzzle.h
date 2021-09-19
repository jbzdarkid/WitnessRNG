#pragma once
#include "forward.h"
#include <string>

struct Cell {
  u8 type;
  u8 x = 0;
  u8 y = 0;

  Dot dot = (Dot)0;
  u8 gap = 0;
  u8 count = 0;
  Line line = (Line)0;
  unsigned short polyshape = 0u;
  int color;

  u8 end;
  bool start = false;

  std::string ToString();
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
  Vector<std::pair<int, int>>* _connections;
  Cell** _grid;
  Masked** _maskedGrid;

  // Properties set and read by Validate() / Solve()
  bool _hasNegations = false;
  bool _hasPolyominos = false;
  Cell* _startPoint = nullptr;
  Cell* _endPoint = nullptr;

  // Non-RNG functions from WP... ish
  Puzzle(int width, int height, bool pillar=false);
  ~Puzzle();
  DELETE_RO3(Puzzle)
  DELETE_RO5(Puzzle)

  // void SetCell(int x, int y, Cell cell);
  Cell* GetCell(int x, int y) const;
  std::pair<int, int> GetSymmetricalPos(int x, int y);
  Cell* GetSymmetricalCell(Cell* cell);
  bool MatchesSymmetricalPos(int x1, int y1, int x2, int y2);
  // A variant of getCell which specifically returns line values,
  // and treats objects as being out-of-bounds
  Line GetLine(int x, int y) const;
  void ClearGrid();

  void _floodFill(int x, int y, Region& region);
  void GenerateMaskedGrid();
  Vector<Region> GetRegions();
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
