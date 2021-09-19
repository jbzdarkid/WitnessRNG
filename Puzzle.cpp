#include <cstdarg>
#include <sstream>

#include "DoubleArray.h"
#include "Puzzle.h"
#include "Random.h"
#include "Utilities.h"

using namespace std;

Puzzle::Puzzle(int width, int height, bool pillar) {
  _origWidth = width;
  _origHeight = height;
  _width = 2*width + (pillar ? 0 : 1);
  _height = 2*height+1;
  _numConnections = (width+1)*height + width*(height+1);

  _grid = NewDoubleArray<Cell>(_width, _height);
  _maskedGrid = NewDoubleArray<u8>(_width, _height);

  for (u8 x=0; x<_width; x++) {
    for (u8 y=0; y<_height; y++) {
      Cell* cell = &_grid[x][y];
      cell->x = x;
      cell->y = y;
      if (x%2 != 1 || y%2 != 1) cell->type = CELL_TYPE_LINE;
    }
  }
  _connections.resize(_numConnections);

  int i = 0;
  // J is the dot index
  for (int j=0; j<(height+1) * (width+1); j++) {
    // WitnessPuzzles grid reference
    int x = (j % (width+1))*2;
    int y = (height - j/(width+1))*2;

    if (y < height*2) { // Do not add a vertical connection for the bottom row
      // _connections[i++] = {j-(width+1), j}; // (Original game reference)
      _connections[i++] = {x, y+1};
    }
    if (x < width*2) { // Do not add a horizontal connection for the last element in the row
      // _connections[i++] = {j, j+1}; // (Original game reference)
      _connections[i++] = {x+1, y};
    }
  }
}

Puzzle::~Puzzle() {
  DeleteDoubleArray(_grid);
  DeleteDoubleArray(_maskedGrid);
}

/*
void Puzzle::SetCell(int x, int y, Cell cell) {
  _grid[x][y] = cell;
}
*/

Cell* Puzzle::GetCell(int x, int y) const {
  x = _mod(x);
  if (!_safeCell(x, y)) return nullptr;
  return &_grid[x][y];
}

pair<int, int> Puzzle::GetSymmetricalPos(int x, int y) {
  if (_symmetry != SYM_NONE) {
    if (_pillar == true) {
      x += _width/2;
      if (_symmetry & SYM_X) {
        x = _width - x;
      }
    } else {
      if (_symmetry & SYM_X) {
        x = (_width - 1) - x;
      }
    }
    if (_symmetry & SYM_Y) {
      y = (_height - 1) - y;
    }
  }
  return {_mod(x), y};
}

Cell* Puzzle::GetSymmetricalCell(Cell* cell) {
  auto [x, y] = GetSymmetricalPos(cell->x, cell->y);
  return &_grid[x][y];
}

bool Puzzle::MatchesSymmetricalPos(int x1, int y1, int x2, int y2) {
  return (_mod(x1) == x2 && y1 == y2);
}

int Puzzle::_mod(int x) const {
  if (_pillar == false) return x;
  return (x + 2 * _width * _height) % _width;
}

bool Puzzle::_safeCell(int x, int y) const {
  if (x < 0 || x >= _width) return false;
  if (y < 0 || y >= _height) return false;
  return true;
}

Line Puzzle::GetLine(int x, int y) const {
  Cell* cell = GetCell(x, y);
  if (cell == nullptr) return Line::None;
  if (cell->type != CELL_TYPE_LINE) return Line::None;
  return cell->line;
}

void Puzzle::ClearGrid() {
  for (int x=0; x<_width; x++) {
    for (int y=0; y<_width; y++) {
      Cell* cell = &_grid[x][y];
      if (x%2 == 1 && y%2 == 1) cell->type = CELL_TYPE_NULL;
      cell->dot = Dot::None;
      cell->gap = GAP_NONE;
      cell->line = Line::None;
      cell->color = 0;
      cell->count = 0;
      cell->polyshape = 0u;

      cell->start = false;
      cell->end = END_NONE;
    }
  }
  _numConnections = (_origWidth+1)*_origHeight + _origWidth*(_origHeight+1);
}

// The grid contains 5 colors:
// null: Out of bounds or already processed
#define MASKED_OOB 0xFF
#define MASKED_PROCESSED 0xFF
// 0: In bounds, awaiting processing, but should not be part of the final region.
#define MASKED_INB_NONCOUNT 0
// 1: In bounds, awaiting processing
#define MASKED_INB_COUNT 1
// 2: Gap-2. After _floodFillOutside, this means "treat normally" (it will be null if oob)
#define MASKED_GAP2 2
// 3: Dot (of any kind), otherwise identical to 1. Should not be flood-filled through (why the f do we need this)
#define MASKED_DOT 3

void Puzzle::_floodFill(int x, int y, Region& region) {
  u8 cell = _maskedGrid[x][y];
  if (cell == MASKED_PROCESSED) return;
  if (cell != MASKED_INB_NONCOUNT) {
    region.emplace_back((u8)x, (u8)y);
  }
  _maskedGrid[x][y] = MASKED_PROCESSED;

  if (y < _height - 1)       _floodFill(x,        y + 1, region);
  if (y > 0)                 _floodFill(x,        y - 1, region);
  if (x < _width - 1)        _floodFill(x + 1,        y, region);
  else if (_pillar != false) _floodFill(0,            y, region);
  if (x > 0)                 _floodFill(x - 1,        y, region);
  else if (_pillar != false) _floodFill(_width-1,     y, region);
}

void Puzzle::GenerateMaskedGrid() {
  // Override all elements with empty lines -- this means that flood fill is just
  // looking for lines with line=0.
  for (int x=0; x<_width; x++) {
    u8* row = _maskedGrid[x];
    int skip = 1;
    if (x%2 == 1) { // Cells are always part of the region
      for (int y = 1; y < _height; y += 2) row[y] = MASKED_INB_COUNT;
      skip = 2; // Skip these cells during iteration
    }

    for (int y=0; y<_height; y+=skip) {
      Cell* cell = &_grid[x][y];
      if (cell->line > Line::None) {
        row[y] = MASKED_PROCESSED; // Traced lines should not be a part of the region
      } else if (cell->gap == GAP_FULL) {
        row[y] = MASKED_GAP2;
      } else if (cell->dot > Dot::None) {
        row[y] = MASKED_DOT;
      } else {
        row[y] = MASKED_INB_COUNT;
      }
    }
  }

  // Starting at a mid-segment startpoint
  if (_startPoint != nullptr && _startPoint->x%2 != _startPoint->y%2) {
    if (false /* _settings.FAT_STARTPOINTS */) {
      // This segment is not in any region (acts as a barrier)
      _maskedGrid[_startPoint->x][_startPoint->y] = MASKED_OOB;
    } else {
      // This segment is part of this region (acts as an empty cell)
      _maskedGrid[_startPoint->x][_startPoint->y] = MASKED_INB_NONCOUNT;
    }
  }

  // Ending at a mid-segment endpoint
  if (_endPoint != nullptr && _endPoint->x%2 != _endPoint->y%2) {
    // This segment is part of this region (acts as an empty cell)
    _maskedGrid[_endPoint->x][_endPoint->y] = MASKED_INB_NONCOUNT;
  }

  // Mark all outside cells as 'not in any region' (aka null)

  /* (Not needed until we have non-square grids)
  // Top and bottom edges
  for (var x=1; x<_width; x+=2) {
    _floodFillOutside(x, 0);
    _floodFillOutside(x, _height - 1);
  }

  // Left and right edges (only applies to non-pillars)
  if (_pillar == false) {
    for (int y=1; y<_height; y+=2) {
      _floodFillOutside(0, y);
      _floodFillOutside(_width - 1, y);
    }
  }
  */
}

vector<Region> Puzzle::GetRegions() {
  vector<Region> regions;
  regions.reserve(5);
  GenerateMaskedGrid();

  for (int x=0; x<_width; x++) {
    for (int y=0; y<_height; y++) {
      if (_maskedGrid[x][y] == MASKED_PROCESSED) continue;

      // If this cell is empty (aka hasn't already been used by a region), then create a new one
      // This will also mark all lines inside the new region as used.
      Region region;
      region.reserve(_width * _height);
      _floodFill(x, y, region);
      regions.emplace_back(move(region));
    }
  }

  return regions;
}

Region Puzzle::GetRegion(int x, int y) {
  Region region;

  x = _mod(x);
  if (!_safeCell(x, y)) return Region();

  GenerateMaskedGrid();
  if (_maskedGrid[x][y] != MASKED_PROCESSED) {
    // If the masked grid hasn't been used at this point, then create a new region.
    // This will also mark all lines inside the new region as used.
    _floodFill(x, y, region);
  }

  return region;
}

void Puzzle::CutRandomEdges(Random& rng, int numCuts) {
  int numConnections = _numConnections; // TW stores the value of this before making cuts.
  for (int i=0; i<numCuts; i++) {
    int rand = rng.Get() % numConnections;

    // In TW, additional connections are added whenever a cut is made. So, we continue if the RNG is larger than the true connection size.
    if (rand >= _connections.size()) continue;

    auto [x, y] = _connections[rand];
    if (_grid[x][y].gap == 0) {
      _numConnections++;
      _grid[x][y].gap = 1;
    }
  }
}

Cell* Puzzle::GetEmptyCell(Random& rng) {
  while (true) {
    int rand = rng.Get() % (_origWidth * _origHeight);

    // This is a guess at converting to WitnessPuzzles grid reference.
    int x = (rand % _origWidth)*2 + 1;
    int y = (_origHeight - rand/_origWidth)*2 - 1;
    Cell* cell = &_grid[x][y];
    if (cell->type == CELL_TYPE_NULL) return cell;
  }
}

ostream& operator<<(ostream& os, const Cell& c) {
  if (c.type == CELL_TYPE_NULL) {
    os << "null";
    return os;
  }

  os << '{';
    if (c.dot != Dot::None) os << "\"dot\": " << dec << (u8)c.dot << ",";
    if (c.gap != 0) os << "\"gap\": " << dec << c.gap << ",";
    if (c.polyshape != 0) os << "\"polyshape\": " << dec << c.polyshape << ",";
    if (c.start) os << "\"start\": true,";
    if (c.end != END_NONE) os << "\"end\": \"" << c.end << "\",";
    if (c.color != 0) os << "\"color\": \"" << c.color << "\",";
    os << "\"type\": \"" << c.type << "\"";
  os << '}';
  return os;
}

ostream& operator<<(ostream& os, const Puzzle& p) {
  os << "{";
    os << "\"width\": " << dec << p._origWidth << ',';
    os << "\"height\": " << dec << p._origHeight << ',';
    os << "\"pillar\": false,";
    os << "\"name\": \"" << p._name << "\",";

    os << "\"grid\": [";
    for (int x=0; x<p._width; x++) {
      os << '[';
      for (int y=0; y<p._height; y++) {
        os << dec << p._grid[x][y].ToString();
        if (y < p._height-1) os << ',';
      }
      os << ']';
      if (x < p._width-1) os << ',';
    }
    os << ']';

  os << "}";
  return os;
}

ostream& operator<<(ostream& os, const Region& r) {
  os << "Region@" << &r;
  return os;
}

#define PRINTF(format, ...) \
  int length = snprintf(nullptr, 0, format, ##__VA_ARGS__); \
  string output(length, '\0'); \
  snprintf(&output[0], output.size() + 1, format, ##__VA_ARGS__); \
  do {} while(0)

const char* IntToString(int i) {
  switch (i) {
    case 0: default:
      return "";
    case 1:
      return "1";
  }
}

std::string Cell::ToString() {
  if (x%2 == 1 && y%2 == 1 && type == CELL_TYPE_NULL) return "null";
  
  const char* typeStr = "";
  if (type == CELL_TYPE_NULL    ) typeStr = ",\"type\":null";
  if (type == CELL_TYPE_LINE    ) typeStr = ",\"type\":\"line\"";
  if (type == CELL_TYPE_SQUARE  ) typeStr = ",\"type\":\"square\"";
  if (type == CELL_TYPE_STAR    ) typeStr = ",\"type\":\"star\"";
  if (type == CELL_TYPE_NEGA    ) typeStr = ",\"type\":\"nega\"";
  if (type == CELL_TYPE_TRIANGLE) typeStr = ",\"type\":\"triangle\"";
  if (type == CELL_TYPE_POLY    ) typeStr = ",\"type\":\"poly\"";
  if (type == CELL_TYPE_YLOP    ) typeStr = ",\"type\":\"ylop\"";

char polyshapeStr[sizeof(R"("polyshape":65535,)")] = {'\0'};
  if (polyshape != 0) sprintf_s(&polyshapeStr[0], sizeof(polyshapeStr), ",\"polyshape\":%hu", polyshape);
  const char* endDir = "";
  if (end == END_LEFT)   endDir = ",\"end\":\"left\"";
  if (end == END_TOP)    endDir = ",\"end\":\"top\"";
  if (end == END_RIGHT)  endDir = ",\"end\":\"right\"";
  if (end == END_BOTTOM) endDir = ",\"end\":\"bottom\"";

  char colorStr[sizeof(R"("color":"#FF00FF",)")] = {'\0'};
  if (color != 0) sprintf_s(&colorStr[0], sizeof(colorStr), ",\"color\":\"#%06x\"", color);

  PRINTF("{\"line\":%d"
    "%s" // type
    "%s%s" // dot
    "%s%s" // gap
    "%s" // start
    "%s" // end
    "%s" // color
    "%s" // polyshape
    "}",
    line,
    typeStr,
    (dot != Dot::None ? ",\"dot\":" : ""), IntToString((u8)dot),
    (gap != 0 ? ",\"gap\":" : ""), IntToString(gap),
    (start != 0 ? ",\"start\":true" : ""),
    endDir,
    colorStr,
    polyshapeStr
  );
  return output;
}
