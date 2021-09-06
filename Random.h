#pragma once
#include <vector>

struct Random {
  int _seed = 0;
  int Get();

  // The bad shuffle
  void ShuffleInt(std::vector<int>& array);

  // The good shuffle
  void ShuffleIntegers(std::vector<int>& arr);
};
