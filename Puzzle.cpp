#include "Puzzle.h"
#include "Random.h"

Puzzle::Puzzle(int width, int height) {
  _width = 2*width + 1;
  _height = 2*height + 1;
  _numConnections = width * height * 2;
  _grid = new Cell*[_width];
  for (int x=0; x<_width; x++) _grid[x] = new Cell[_height];
  _connections.resize(_numConnections);

  int i = 0;
  // J is the dot index
  for (int j=0; j<(height+1) * (width+1); j++) {
    // WitnessPuzzles grid reference
    int x = (j % (width+1))*2;
    int y = height*2 - (j / (width+1))*2;

    if (x < width*2) { // Do not add a horizontal connection for the last element in the row
      // _connections[i++] = {j, j+1}; // (Original game reference)
      _connections[i++] = {x+1, y};
    }
    if (y < height*2) { // Do not add a vertical connection for the bottom row
      // _connections[i++] = {j-(width+1), j}; // (Original game reference)
      _connections[i++] = {x, y+1};
    }
  }
}

Puzzle Puzzle::GeneratePolyominos() {
  Puzzle p = Puzzle(4, 4);
  p._grid[0][4].start = true;
  p._grid[4][0].end = "right";

  return p;
}

void Puzzle::SetCell(int x, int y, Cell cell) {
  _grid[x][y] = cell;
}

const Cell* Puzzle::GetCell(int x, int y) {
  return &_grid[x][y];
}

void Puzzle::CutRandomEdges(Random& rng, int numCuts) {
  for (int i=0; i<numCuts; i++) {
    int rand = rng.Get() % _numConnections;

    // In TW, additional connections are added whenever a cut is made.
    if (rand > _connections.size()) continue;

    auto [x, y] = _connections[rand];
    _numConnections++;
    _grid[x][y].gap = 1;
  }
}
