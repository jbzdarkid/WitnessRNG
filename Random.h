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
  Puzzle* GenerateSimpleMaze();
  Puzzle* GenerateHardMaze();
  Puzzle* GenerateStones();
  Puzzle* GeneratePedestal();
  Puzzle* GeneratePolyominos(bool rerollOnImpossible);
  Puzzle* GenerateStars();
  Puzzle* GenerateSymmetry();
  Puzzle* GenerateTriple2(bool shouldBeSolvable);
  Puzzle* GenerateTriple3(bool shouldBeSolvable);
  Puzzle* GenerateTriangles(u8 count);
  Puzzle* GenerateDotsPillar();
  Puzzle* GenerateStonesPillar();
  static bool IsSolvable(int seed);
  static bool IsSolvable(Puzzle* p);

private:
  int _seed = 0;
};
