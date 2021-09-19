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

  unsigned short RandomPolyshape();

  Puzzle* GeneratePolyominos(bool rerollOnImpossible, bool abortOnStarsFailure=false);

private:
  int _seed = 0;
};
