#include "stdafx.h"

#include <sstream>

Puzzle::Puzzle(u8 width, u8 height, bool pillar) {
  _origWidth = width;
  _origHeight = height;
  _width = 2*width + (pillar ? 0 : 1);
  _height = 2*height+1;
  _numConnections = (width+1)*height + width*(height+1);

  _grid = new NArray<Cell>(_width, _height);
  _grid->Fill(Cell{});
  _maskedGrid = new NArray<Masked>(_width, _height);
  _maskedGrid->Fill(Masked::Uncounted);

  for (u8 x=0; x<_width; x++) {
    for (u8 y=0; y<_height; y++) {
      Cell* cell = &_grid->Get(x, y);
      cell->x = x;
      cell->y = y;
      if (x%2 != 1 || y%2 != 1) cell->type = Type::Line;
    }
  }
  // Extra connections because we'll probably add a connection for the end
  _connections = new Vector<u8>(_numConnections * 2 + 2);

  // J is the dot index
  for (u8 j=0; j<(height+1) * (width+1); j++) {
    // WitnessPuzzles grid reference
    u8 x = (j % (width+1))*2;
    u8 y = (height - j/(width+1))*2;

    if (y < height*2) { // Do not add a vertical connection for the bottom row
      // _connections->Emplace({j-(width+1), j}); // (Original game reference)
      // _connections->Emplace({x, (u8)(y+1)});
      _connections->UnsafePush(x);
      _connections->UnsafePush(y+1);
    }
    if (x < width*2) { // Do not add a horizontal connection for the last element in the row
      // _connections->Emplace({j, j+1}); // (Original game reference)
      // _connections->Emplace({(u8)(x+1), y});
      _connections->UnsafePush(x+1);
      _connections->UnsafePush(y);
    }
  }
}

Puzzle::~Puzzle() {
  if (_grid) delete _grid;
  if (_maskedGrid) delete _maskedGrid;
  delete _connections;
}

Cell* Puzzle::GetCell(s8 x, s8 y) const {
  x = _mod(x);
  if (!_safeCell(x, y)) return nullptr;
  return &_grid->Get(x, y);
}

void Puzzle::SetStart(s8 x, s8 y) {
  Cell* cell = GetCell(x, y);
  assert(cell);
  assert(!cell->start);
  cell->start = true;

  if (_symmetry != SYM_NONE) {
    Cell* sym = GetSymmetricalCell(cell);
    assert(sym);
    assert(!sym->start);
    sym->start = true;
    _connections->Push(sym->x);
    _connections->Push(sym->y);
    _numConnections++;
  }
}

void Puzzle::SetEnd(s8 x, s8 y, End dir) {
  Cell* cell = GetCell(x, y);
  assert(cell);
  assert(cell->end == End::None);
  cell->end = dir;
  _connections->Push(x);
  _connections->Push(y);
  _numConnections++;

  if (_symmetry != SYM_NONE) {
    Cell* sym = GetSymmetricalCell(cell);
    assert(sym);
    assert(sym->end == End::None);
    sym->end = (End)(5 - (int)dir);
    _connections->Push(sym->x);
    _connections->Push(sym->y);
    _numConnections++;
  }
}

pair<u8, u8> Puzzle::GetSymmetricalPos(s8 x, s8 y) {
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
  return &_grid->Get(x, y);
}

bool Puzzle::MatchesSymmetricalPos(s8 x1, s8 y1, s8 x2, s8 y2) {
  return (y1 == y2 && _mod(x1) == x2);
}

u8 Puzzle::_mod(s8 x) const {
  if (_pillar == false) return x;
  return (x + 2 * _width * _height) % _width;
}

bool Puzzle::_safeCell(s8 x, s8 y) const {
  if (x < 0 || x >= _width) return false;
  if (y < 0 || y >= _height) return false;
  return true;
}

Line Puzzle::GetLine(s8 x, s8 y) const {
  Cell* cell = GetCell(x, y);
  if (cell == nullptr) return Line::None;
  if (cell->type != Type::Line) return Line::None;
  return cell->line;
}

void Puzzle::ClearGrid(bool linesOnly) {
  for (u8 x=0; x<_width; x++) {
    for (u8 y=0; y<_width; y++) {
      Cell* cell = &_grid->Get(x, y);
      cell->line = Line::None;
      if (!linesOnly) {
        if (x % 2 == 1 && y % 2 == 1) cell->type = Type::Null;
        cell->dot = Dot::None;
        cell->gap = Gap::None;
        cell->color = 0;
        cell->count = 0;
        cell->polyshape = 0u;

        cell->start = false;
        cell->end = End::None;
      }
    }
  }
  _numConnections = (_origWidth+1)*_origHeight + _origWidth*(_origHeight+1);
}

void Puzzle::_floodFill(u8 x, u8 y, Region& region) {
  Masked cell = _maskedGrid->Get(x, y);
  if (cell == Masked::Processed) return;
  if (cell != Masked::Uncounted) {
    region.Emplace(&_grid->Get(x, y));
  }
  _maskedGrid->Get(x, y) = Masked::Processed;

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
  for (u8 x=0; x<_width; x++) {
    Masked* row = _maskedGrid->GetRow(x);
    u8 skip = 1;
    if (x%2 == 1) { // Cells are always part of the region
      for (u8 y = 1; y < _height; y += 2) row[y] = Masked::Counted;
      skip = 2; // Skip these cells during iteration
    }

    for (u8 y=0; y<_height; y+=skip) {
      Cell* cell = &_grid->Get(x, y);
      if (cell->line != Line::None) {
        row[y] = Masked::Processed; // Traced lines should not be a part of the region
      } else if (cell->gap == Gap::Full) {
        row[y] = Masked::Gap2;
      } else if (cell->dot != Dot::None) {
        row[y] = Masked::Dot;
      } else {
        row[y] = Masked::Counted;
      }
    }
  }

  // Starting at a mid-segment startpoint
  if (_startPoint != nullptr && _startPoint->x%2 != _startPoint->y%2) {
    if (false /* _settings.FAT_STARTPOINTS */) {
      // This segment is not in any region (acts as a barrier)
      _maskedGrid->Get(_startPoint->x, _startPoint->y) = Masked::OutOfBounds;
    } else {
      // This segment is part of this region (acts as an empty cell)
      _maskedGrid->Get(_startPoint->x, _startPoint->y) = Masked::Uncounted;
    }
  }

  // Ending at a mid-segment endpoint
  if (_endPoint != nullptr && _endPoint->x%2 != _endPoint->y%2) {
    // This segment is part of this region (acts as an empty cell)
    _maskedGrid->Get(_endPoint->x, _endPoint->y) = Masked::Uncounted;
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

void Puzzle::GetRegions(Vector<Region>& regions, LinearAllocator<Cell*>& alloc) {
  regions.Resize(0);
  GenerateMaskedGrid();

  // A limit for the total size of all regions -- at least this way, we won't allocate all the regions as large as possible.
  int remainingRegionSize = _width * _height;

  for (u8 x=0; x<_width; x++) {
    for (u8 y=0; y<_height; y++) {
      if (_maskedGrid->Get(x, y) == Masked::Processed) continue;

      // If this cell is empty (aka hasn't already been used by a region), then create a new one
      // This will also mark all lines inside the new region as used.
      Region region(remainingRegionSize, alloc);
      _floodFill(x, y, region);
      remainingRegionSize -= region.Size();
      regions.Emplace(move(region));
    }
  }
}

Region Puzzle::GetRegion(s8 x, s8 y) {
  Region region(_width * _height);

  Cell* cell = GetCell(x, y);
  if (cell == nullptr) return region;
  x = cell->x; // Hacky, substitute for calling _mod.

  GenerateMaskedGrid();
  if (_maskedGrid->Get(x, y) != Masked::Processed) {
    // If the masked grid hasn't been used at this point, then create a new region.
    // This will also mark all lines inside the new region as used.
    _floodFill(x, y, region);
  }

  return region;
}

u64 Puzzle::GetPolyishFromMaskedGrid(u8 rotation, bool flip) {
  u64 polyish = 0;
  for (u8 x=1; x<_width; x+=2) {
    Masked* col = _maskedGrid->GetRow(x);
    for (u8 y=1; y<_height; y+=2) {
      if (col[y] != Masked::Processed) continue;

      u8 newX = (x - 1) / 2;
      u8 newY = (y - 1) / 2;
      if (flip) {
        newY = 7 - newY;
      }
      for (int j=0; j<rotation % 4; j++) {
        u8 tmp = newX;
        newX = newY;
        newY = 7 - tmp;
      }

      assert(0 <= newX && newX < 8);
      assert(0 <= newY && newY < 8);

      polyish |= (u64)1 << (newX * 8 + newY);
    }
  }

  // Each row is represented by a u8 (00 - FF). So, halve the number until any column is odd.
  while ((polyish & 0x0101'0101'0101'0101) == 0) polyish >>= 1;
  // Each column is represented by a u8 mask (01 - 80). So, shift until rightmost column has something in it.
  while ((polyish & 0x0000'0000'0000'00FF) == 0) polyish >>= 8;

  return polyish;
}

void Puzzle::CutRandomEdges(Random& rng, u8 numCuts) {
  u8 numConnections = _numConnections; // TW stores the value of this before making cuts, so that we only attempt to cut valid edges.
  for (int i=0; i<numCuts; i++) {
    int rand = rng.Get() % numConnections;

    u8 x = _connections->At(rand*2);
    u8 y = _connections->At(rand*2 + 1);
    if (_grid->Get(x, y).gap == Gap::None) {
      _numConnections++;
      _grid->Get(x, y).gap = Gap::Break;
    }
  }
}

void Puzzle::AddRandomDots(Random& rng, u8 numDots, Dot color) {
  u8 numConnections = _numConnections; // TW stores the value of this before making cuts.
  for (int i = 0; i < numDots; i++) {
    int rand = rng.Get() % numConnections;

    // In TW, additional connections are added whenever a cut is made. So, we continue if the RNG is larger than the true connection size.
    if (rand * 2 >= _connections->Size()) continue;

    u8 x = _connections->At(rand * 2);
    u8 y = _connections->At(rand * 2 + 1);
    if (_grid->Get(x, y).dot == Dot::None) {
      _numConnections++;
      _grid->Get(x, y).dot = color;
    }
  }
}

Cell* Puzzle::GetEmptyCell(Random& rng) {
  while (true) {
    Cell* cell = GetRandomCell(rng);
    if (cell->type == Type::Null) return cell;
  }
}

Cell* Puzzle::GetRandomCell(Random& rng) {
  int rand = rng.Get() % (_origWidth * _origHeight);

  // This converts to WitnessPuzzles grid references.
  int x = (rand % _origWidth)*2 + 1;
  int y = (_origHeight - rand/_origWidth)*2 - 1;
  assert((u8)x == x);
  assert((u8)y == y);
  return &_grid->Get((u8)x, (u8)y);
}

bool Puzzle::TestStonesEarlyFail() {
  for (u8 x = 1; x < _width - 2; x += 2) {
    for (u8 y = 1; y < _height - 2; y += 2) {
      Cell* c1 = &_grid->Get(x+0, y+0);
      Cell* c2 = &_grid->Get(x+0, y+2);
      Cell* c3 = &_grid->Get(x+2, y+0);
      Cell* c4 = &_grid->Get(x+2, y+2);

      if (c1->type == Type::Square && c2->type == Type::Square && c1->color != c2->color) {
        if (c3->type == Type::Square && c3->color != c1->color && c3->color != c2->color) return true;
        if (c4->type == Type::Square && c4->color != c1->color && c4->color != c2->color) return true;
      } else if (c2->type == Type::Square && c3->type == Type::Square && c2->color != c3->color) {
        if (c4->type == Type::Square && c4->color != c2->color && c4->color != c3->color) return true;
        if (c1->type == Type::Square && c1->color != c2->color && c1->color != c3->color) return true;
      } else if (c3->type == Type::Square && c4->type == Type::Square && c3->color != c4->color) {
        if (c1->type == Type::Square && c1->color != c3->color && c1->color != c4->color) return true;
        if (c2->type == Type::Square && c2->color != c3->color && c2->color != c4->color) return true;
      } else if (c4->type == Type::Square && c1->type == Type::Square && c4->color != c1->color) {
        if (c2->type == Type::Square && c2->color != c4->color && c2->color != c1->color) return true;
        if (c3->type == Type::Square && c3->color != c4->color && c3->color != c1->color) return true;
      }
    }
  }

  return false;
}

#define PRINTF(variable, format, ...) \
  int length = snprintf(nullptr, 0, format, ##__VA_ARGS__); \
  string variable = string(length, '\0'); \
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
  if (x%2 == 1 && y%2 == 1 && type == Type::Null) return "null";

  const char* typeStr = "";
  if (type == Type::Null    ) typeStr = ",\"type\":null";
  if (type == Type::Line    ) typeStr = ",\"type\":\"line\"";
  if (type == Type::Square  ) typeStr = ",\"type\":\"square\"";
  if (type == Type::Star    ) typeStr = ",\"type\":\"star\"";
  if (type == Type::Nega    ) typeStr = ",\"type\":\"nega\"";
  if (type == Type::Triangle) typeStr = ",\"type\":\"triangle\"";
  if (type == Type::Poly    ) typeStr = ",\"type\":\"poly\"";
  if (type == Type::Ylop    ) typeStr = ",\"type\":\"ylop\"";

  char polyshapeStr[sizeof(R"("polyshape":65535,)")] = {'\0'};
  if (polyshape != 0) sprintf_s(&polyshapeStr[0], sizeof(polyshapeStr), ",\"polyshape\":%hu", polyshape);
  const char* endDir = "";
  if (end == End::Left)   endDir = ",\"end\":\"left\"";
  if (end == End::Top)    endDir = ",\"end\":\"top\"";
  if (end == End::Right)  endDir = ",\"end\":\"right\"";
  if (end == End::Bottom) endDir = ",\"end\":\"bottom\"";

  char colorStr[sizeof(R"("color":"#FF00FF",)")] = {'\0'};
  if (color != 0) sprintf_s(&colorStr[0], sizeof(colorStr), ",\"color\":\"#%06x\"", color);

  PRINTF(output, "{\"line\":%d"
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
    (gap != Gap::None ? ",\"gap\":" : ""), IntToString((u8)gap),
    (start != false ? ",\"start\":true" : ""),
    endDir,
    colorStr,
    polyshapeStr
  );
  return output;
}

string Puzzle::ToString() {
  stringstream grid;
  grid << '[';
  for (u8 x=0; x<_width; x++) {
    grid << '[';
    for (u8 y=0; y<_height; y++) {
      grid << _grid->Get(x, y).ToString();
      if (y < _height-1) grid << ',';
    }
    grid << ']';
    if (x < _width-1) grid << ',';
  }
  grid << ']';

  string gridStr = grid.str();

  PRINTF(output, R"({"width":%d,"height":%d,"pillar":%s,"name":"%s","grid":%s})" "\n",
    _width, _height, (_pillar ? "true" : "false"), _name.c_str(), gridStr.c_str());
  return output;
}

void Puzzle::LogGrid() {
  for (u8 y=0; y<_height; y++) {
    for (u8 x=0; x<_width; x++) {
      Cell* cell = &_grid->Get(x, y);
      if (cell == nullptr)                    cout << ' ';
      else if (cell->type == Type::Null)      cout << ' ';
      else if (cell->type == Type::Line) {
        if (cell->gap > Gap::None)            cout << ' ';
        else if (cell->dot > Dot::None)       cout << 'X';
        else if (cell->start == true)         cout << 'S';
        else if (cell->end != End::None)      cout << 'E';
        else if (cell->line == Line::None)    cout << '.';
        else if (cell->line == Line::Black)   cout << '#';
        else if (cell->line == Line::Blue)    cout << '#';
        else if (cell->line == Line::Yellow)  cout << 'o';
      }
      else                                    cout << '?';
    }
    cout << endl;
  }
}
