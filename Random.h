#pragma once
#include "forward.h"
#include <vector>

class Random {
public:
  int Get();
  int Peek();
  void Set(int rng);

  // The bad shuffle
  void ShuffleInt(std::vector<int>& array);

  // The good shuffle
  void ShuffleIntegers(std::vector<int>& arr);

  unsigned short RandomPolyshape();

  Puzzle* GeneratePolyominos(bool rerollOnImpossible, bool abortOnStarsFailure=false);

private:
  int _seed = 0;
};
