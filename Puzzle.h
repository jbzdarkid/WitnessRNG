#pragma once
#include "forward.h"
#include <string>

struct Cell {
  Type type = (Type)0;
  u8 x = 0;
  u8 y = 0;

  Dot dot = (Dot)0;
  Gap gap = (Gap)0;
  u8 count = 0;
  Line line = (Line)0;
  u16 polyshape = 0u;
  int color = 0;

  std::string ToString();

private:
  End end = (End)0;
  bool start = false;

  friend class Puzzle;
  friend class Solver;
};

class Puzzle {
public:
  u8 _origWidth = 0;
  u8 _origHeight = 0;
  u8 _height = 0;
  u8 _width = 0;
  u8 _numConnections = 0;
  u8 _symmetry = 0;
  NArray<Masked>* _maskedGrid;
  Vector<u8>* _connections;
  std::string _name;
  bool _pillar = false;

  // Properties set and read by Validate() / Solve()
  bool _hasNegations = false;
  bool _hasPolyominos = false;
  Cell* _startPoint = nullptr;
  Cell* _endPoint = nullptr;

  // Non-RNG functions from WP... ish
  Puzzle(u8 width, u8 height, bool pillar=false);
  ~Puzzle();
  DELETE_RO3(Puzzle)
  DELETE_RO5(Puzzle)

  // Start/end setters, for safety reasons
  void SetStart(s8 x, s8 y);
  void SetEnd(s8 x, s8 y, End dir);

  Cell* GetCell(s8 x, s8 y) const;
  std::pair<u8, u8> GetSymmetricalPos(s8 x, s8 y);
  Cell* GetSymmetricalCell(Cell* cell);
  bool MatchesSymmetricalPos(s8 x1, s8 y1, s8 x2, s8 y2);
  // A variant of getCell which specifically returns line values,
  // and treats objects as being out-of-bounds
  Line GetLine(s8 x, s8 y) const;
  void ClearGrid(bool linesOnly = false);

  void _floodFill(u8 x, u8 y, Region& region);
  void GenerateMaskedGrid();
  void GetRegions(Vector<Region>& regions);
  Region GetRegion(s8 x, s8 y);
  // Works for up to an 8x8 region
  u64 GetPolyishFromMaskedGrid(u8 rotation, bool flip);

  std::string ToString(); // Can be imported into TW
  void LogGrid();

  // RNG functions (from TW)
  void CutRandomEdges(Random& rng, u8 numCuts);
  void AddRandomDots(Random& rng, u8 numDots, Dot color = (Dot)1 /* Dot::Black */);
  Cell* GetEmptyCell(Random& rng);
  Cell* GetRandomCell(Random& rng);
  // Non-RNG, also from TW
  bool TestStonesEarlyFail();

private:
  NArray<Cell>* _grid;

  u8 _mod(s8 x) const;
  bool _safeCell(s8 x, s8 y) const;

  friend class Solver;
  friend class Validator;
};
