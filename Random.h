#pragma once
#include "forward.h"

class Random {
public:
  int Get();
  int Peek();
  void Set(int rng);

  // The bad shuffle
  void ShuffleInt(Vector<int>& array);

  // The good shuffle
  void ShuffleIntegers(Vector<int>& arr);

  u16 RandomPolyshape();

  int CheckStarsFailure();

  Vector<Puzzle*> GenerateChallenge();
  Puzzle* GenerateEasyMaze(bool rerollOnImpossible);
  Puzzle* GenerateHardMaze(bool rerollOnImpossible);
  Puzzle* GenerateStones(bool rerollOnImpossible);
  Puzzle* GeneratePedestal(bool rerollOnImpossible);
  Puzzle* GenerateSymmetry(bool rerollOnImpossible);
  Puzzle* GeneratePolyominos(bool rerollOnImpossible);
  Puzzle* GenerateStars(bool rerollOnImpossible);
  Puzzle* GenerateTriple2(bool solvable);
  Puzzle* GenerateTriple3(bool solvable);
  Puzzle* GenerateTriangles(u8 count, bool rerollOnImpossible);
  Puzzle* GenerateDotsPillar(bool rerollOnImpossible);
  Puzzle* GenerateStonesPillar(bool rerollOnImpossible);
  static bool IsSolvable(int seed);

private:
  int _seed = 0;
};
