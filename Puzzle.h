#pragma once
#include <string>
#include <vector>
struct Random;

struct Cell {
  std::string type;
  int dot = 0;
  int gap = 0;
  unsigned short polyshape = 0u;
  bool start = false;

  std::string end;
  std::string color;
};

struct Puzzle {
  int _height = 0;
  int _width = 0;
  int _numConnections = 0;
  std::vector<std::tuple<int, int>> _connections;
  Cell** _grid;

  Puzzle(int _width, int _height);
  static Puzzle GeneratePolyominos(Random& rng);

  void SetCell(int x, int y, Cell cell);
  const Cell* GetCell(int x, int y);

  void CutRandomEdges(Random& rng, int numCuts);
  void AddRandomDots(Random& rng, int numDots);
  std::tuple<int, int> GetEmptyCell(Random& rng);
};

std::ostream& operator<<(std::ostream& os, const Puzzle& dt);