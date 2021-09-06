#include "Puzzle.h"
#include "Random.h"
using namespace std;

Puzzle::Puzzle(int width, int height) {
  _width = width;
  _height = height;
  _numConnections = (width+1)*height + width*(height+1);
  _grid = new Cell*[2*width+1];
  for (int x=0; x<2*width+1; x++) _grid[x] = new Cell[2*height+1];
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
  p._grid[0][8].start = true;
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

  return p;
}

void Puzzle::SetCell(int x, int y, Cell cell) {
  _grid[x][y] = cell;
}

const Cell* Puzzle::GetCell(int x, int y) {
  return &_grid[x][y];
}

void Puzzle::CutRandomEdges(Random& rng, int numCuts) {
  int numConnections = _numConnections; // TW stores the value of this before making cuts.
  for (int i=0; i<numCuts; i++) {
    int rand = rng.Get() % numConnections;

    // In TW, additional connections are added whenever a cut is made. So, we continue if the RNG is larger than the true connection size.
    if (rand > _connections.size()) continue;

    auto [x, y] = _connections[rand];
    if (_grid[x][y].gap == 0) {
      _numConnections++;
      _grid[x][y].gap = 1;
    }
  }
}

tuple<int, int> Puzzle::GetEmptyCell(Random& rng) {
  while (true) {
    int rand = rng.Get() % (_width * _height);

    // This is a guess at converting to WitnessPuzzles grid reference.
    int x = (rand % _width)*2 + 1;
    int y = (_height - rand/_width)*2 - 1;
    if (_grid[x][y].type.empty()) return {x, y};
  }
}
