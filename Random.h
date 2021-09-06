#pragma once
#include <vector>

struct Random {
  int _seed = 0;
  int Get();

  // The bad shuffle
  /*
  template<typename T>
  void Shuffle(std::vector<T>& array) {
    size_t size = array.size();
    for (int i=0; i<size; i++) {
      T* first = &array[Get() % size];
      T* second = &array[Get() % size];
      int tmp = *first;
      *first = *second;
      *second = tmp;
    }
  }
  */

  void ShuffleIntegers(std::vector<int>& array);
};
