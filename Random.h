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
  Puzzle* GeneratePolyominos(bool rerollOnImpossible);
  static bool IsSolvable(int seed);

private:
  int _seed = 0;
};
