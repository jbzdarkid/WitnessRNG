#pragma once
#include "forward.h"

class Solver {
public:
  Solver(Puzzle* puzzle_);
  ~Solver();

  bool IsSolvable();
  // Generates a solution via DFS recursive backtracking
  Vector<Path> Solve(int maxSolutions = 10'000);

private:
  void TailRecurse(Cell* cell);
  // Note: Most mechanics are NP (or harder), so don't feel bad about solving them by brute force.
  // https://arxiv.org/pdf/1804.10193.pdf
  void SolveLoop(int x, int y, Vector<Path>& solutionPaths);

  Puzzle* puzzle;
  Path* path;
  int pathSize = 0;
  int MAX_SOLUTIONS = 0;
  int numEndpoints = 0;
  bool doPruning = false;
};

