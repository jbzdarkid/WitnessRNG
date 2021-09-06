#include "Puzzle.h"

Puzzle::Puzzle(int width, int height) {
  this->width = width;
  this->height = height;
  this->_grid = new Cell*[width];
  for (int x=0; x<width; x++) this->_grid[x] = new Cell[height];
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
