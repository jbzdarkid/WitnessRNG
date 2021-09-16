#include <cstdarg>
#include <sstream>

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
  _grid = new Cell*[_width];
  for (int x=0; x<_width; x++) {
    _grid[x] = new Cell[_height];
    for (int y = 0; y < _height; y++) {
      _grid[x][y].x = x;
      _grid[x][y].y = y;
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

Puzzle Puzzle::GeneratePolyominos(Random& rng) {
  Puzzle p = Puzzle(4, 4);
  p._name = "Random polyominos #" + std::to_string(rng._seed);
  p._grid[0][8].type = "line";
  p._grid[0][8].start = true;
  p._grid[8][0].type = "line";
  p._grid[8][0].end = "right"; p._numConnections++;

  rng.Get();
  vector<int> colors = {0, 1, 2, 3, 4};
  vector<string> colorNames = {"orange", "purple", "green", "red", "teal"};
  rng.ShuffleInt(colors);

  while (true) {
    auto [x1, y1] = p.GetEmptyCell(rng);
    auto [x2, y2] = p.GetEmptyCell(rng);

    // Manhattan Distance of 3 or more
    if (abs(x1 - x2) + abs(y1 - y2) < 6) continue;

    p._grid[x1][y1].type = "star";
    p._grid[x1][y1].color = colorNames[colors[0]];
    p._grid[x2][y2].type = "star";
    p._grid[x2][y2].color = colorNames[colors[0]];
    break;
  }

  p.CutRandomEdges(rng, 8);

  unsigned short polyshape1 = rng.RandomPolyshape();
  unsigned short polyshape2 = rng.RandomPolyshape();
  auto [x1, y1] = p.GetEmptyCell(rng);
  auto [x2, y2] = p.GetEmptyCell(rng);
  p._grid[x1][y1].type = "poly";
  p._grid[x1][y1].color = colorNames[colors[1]];
  p._grid[x1][y1].polyshape = polyshape1;
  p._grid[x2][y2].type = "poly";
  p._grid[x2][y2].color = colorNames[colors[1]];
  p._grid[x2][y2].polyshape = polyshape2;

  return p;
}

void Puzzle::SetCell(int x, int y, Cell cell) {
  _grid[x][y] = cell;
}

Cell* Puzzle::GetCell(int x, int y) {
  x = _mod(x);
  if (!_safeCell(x, y)) return nullptr;
  return &_grid[x][y];
}

int Puzzle::_mod(int x) {
  if (_pillar == false) return x;
  return (x + 2 * _width * _height) % _width;
}

bool Puzzle::_safeCell(int x, int y) {
  if (x < 0 || x >= _width) return false;
  if (y < 0 || y >= _height) return false;
  return true;
}

int Puzzle::GetLine(int x, int y) {
  Cell* cell = GetCell(x, y);
  if (cell == nullptr) return LINE_NONE;
  if (cell->type != "line") return LINE_NONE;
  return cell->line;
}

vector<Region> Puzzle::GetRegions() {
  return {}; // This function is big and complicated... but can probably be made simpler for this project.
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
      _grid[x][y].type = "line";
      _grid[x][y].gap = 1;
    }
  }
}

tuple<int, int> Puzzle::GetEmptyCell(Random& rng) {
  while (true) {
    int rand = rng.Get() % (_origWidth * _origHeight);

    // This is a guess at converting to WitnessPuzzles grid reference.
    int x = (rand % _origWidth)*2 + 1;
    int y = (_origHeight - rand/_origWidth)*2 - 1;
    if (_grid[x][y].type.empty()) return {x, y};
  }
}

ostream& operator<<(ostream& os, const Cell& c) {
  if (c.type.empty()) {
    os << "null";
    return os;
  }

  os << '{';
    if (c.dot != 0) os << "\"dot\": " << dec << c.dot << ",";
    if (c.gap != 0) os << "\"gap\": " << dec << c.gap << ",";
    if (c.polyshape != 0) os << "\"polyshape\": " << dec << c.polyshape << ",";
    if (c.start) os << "\"start\": true,";
    if (!c.end.empty()) os << "\"end\": \"" << c.end << "\",";
    if (!c.color.empty()) os << "\"color\": \"" << c.color << "\",";
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
        os << dec << p._grid[x][y].ToString(x, y);
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

std::string Cell::ToString(int x, int y) {
  if (x%2 == 1 && y%2 == 1 && type.empty()) return "null";

  char polyshapeStr[sizeof(R"("polyshape":65535,)")] = {'\0'};
  if (polyshape != 0) sprintf_s(&polyshapeStr[0], sizeof(polyshapeStr), ",\"polyshape\":%hu", polyshape);

  PRINTF("{\"type\":\"%s\",\"line\":0"
    "%s%s" // dot
    "%s%s" // gap
    "%s" // start
    "%s%s%s" // end
    "%s%s%s" // color
    "%s" // polyshape
    "}",
    type.empty() ? "line" : type.c_str(),
    (dot != 0 ? ",\"dot\":" : ""), IntToString(dot),
    (gap != 0 ? ",\"gap\":" : ""), IntToString(gap),
    (start != 0 ? ",\"start\":true" : ""),
    (!end.empty() ? ",\"end\":\"" : ""), end.c_str(), (!end.empty() ? "\"" : ""),
    (!color.empty() ? ",\"color\":\"" : ""), color.c_str(), (!color.empty() ? "\"" : ""),
    polyshapeStr
  );
  return output;
}
