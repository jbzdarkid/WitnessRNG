#pragma once
#include <string>

struct Cell {
  std::string type;
  int dot = 0;
  int gap = 0;
  bool start = false;
  std::string end;
};

struct Puzzle {
  int height = 0;
  int width = 0;
  Cell** _grid;

  Puzzle(int width, int height);
  static Puzzle GeneratePolyominos();

  void SetCell(int x, int y, Cell cell);
  const Cell* GetCell(int x, int y);

  void CutRandomEdges(int numCuts);
  void AddRandomDots(int numDots);

};
