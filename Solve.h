#pragma once
#include "forward.h"

class Solver {
public:
  Solver(Puzzle* puzzle_);
  ~Solver();

  // Generates a solution via DFS recursive backtracking
  Vector<Path> Solve(int maxSolutions = 10'000);

private:
  void TailRecurse(Cell* cell);
  // Note: Most mechanics are NP (or harder), so don't feel bad about solving them by brute force.
  // https://arxiv.org/pdf/1804.10193.pdf
  void SolveLoop(s8 x, s8 y, Vector<Path>& solutionPaths, u8 numEndpoints);

  Puzzle* puzzle;
  Path* path;
  int MAX_SOLUTIONS = 0;
  bool doPruning = false;
  struct EarlyExitData {
    bool hasEverLeftEdge;
    s8 x1; s8 y1; bool isEdge1;
    s8 x2; s8 y2; bool isEdge2;
  };
  EarlyExitData earlyExitData;
};

