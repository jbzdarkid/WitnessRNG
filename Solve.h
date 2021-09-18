#pragma once
#include <vector>

struct Cell;
class Puzzle;

using u8 = unsigned char;
using Path = u8*;

class Solver {
public:
  Solver(Puzzle* puzzle_);

  // Generates a solution via DFS recursive backtracking
  std::vector<Path> Solve(int maxSolutions = 10'000);

private:
  void TailRecurse(Cell* cell);
  void PushPath(u8 value);
  void SetLastPath(u8 value);
  u8 PopPath();
  // Note: Most mechanics are NP (or harder), so don't feel bad about solving them by brute force.
  // https://arxiv.org/pdf/1804.10193.pdf
  void SolveLoop(int x, int y);

  Puzzle* puzzle;
  Path path;
  int pathSize = 0;
  std::vector<Path> solutionPaths;
  int MAX_SOLUTIONS = 0;
  int numEndpoints = 0;
  bool doPruning = false;
};

