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

  Puzzle* GeneratePolyominos(bool rerollOnImpossible, bool abortOnStarsFailure=false, u8* validSeeds=nullptr);

private:
  int _seed = 0;
};
