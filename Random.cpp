#include "Random.h"
using namespace std;

constexpr int m_prime = 0x7FFFFFFF; // 2^31 - 1, a mersenne prime

int Random::Get() {
  if (_seed == 0) {
    _seed = 111111;
  }
  // This first operation is relatively low entropy, since the multiplicand is close to -2^31 (-2'147'483'648)
  // Thus, this operation is approximately (int32_t)((float)this._current_rng / -2.053f);
  int nextRng = (int)(_seed * 0xFFFF'FFFF'834E'0B5FL / 0x1'0000'0000L);
  nextRng = (_seed + nextRng) / 0x1'0000;
  // This is also kind of weird. It takes a different action if nextRng is even or odd.
  // This operation is also expressible as (nextRng % 2 == 0 : -nextRng ? 2^31 - nextRng)
  nextRng = -nextRng * m_prime;
  nextRng = nextRng + _seed * 16807; // 16807 == 7^5
  if (nextRng <= 0) { // Rarely true in practice
    nextRng = nextRng + m_prime; // Makes the result positive, even though the output is unsigned.
  }
  _seed = nextRng;
  return nextRng;
}

void Random::ShuffleInt(std::vector<int>& arr) {
  size_t size = arr.size();
  for (size_t i=0; i<size; i++) {
    size_t rng1 = Get() % size;
    size_t rng2 = Get() % size;
    int tmp = arr[rng1];
    arr[rng1] = arr[rng2];
    arr[rng2] = tmp;
  }
}

void Random::ShuffleIntegers(vector<int>& arr) {
  for (size_t size = arr.size(); size > 1; size--) {
    size_t rng = Get() % size;
    int tmp = arr[rng];
    arr[rng] = arr[size-1];
    arr[size-1] = tmp;
  }
}

unsigned int RotatePolyshape(unsigned int polyshape) {
  unsigned int newshape = 0;
  for (int x=0; x<4; x++) {
    for (int y=0; y<4; y++) {
      if (polyshape & (1 << (x*4 + y))) newshape |= 1 << (y*4 + 3-x);
    }
  }
  return newshape;
}

unsigned int Random::RandomPolyshape() {
  int cursor_x = 0;
  int cursor_y = 0;
  unsigned short polyshape = 1;

  for (int size = (Get() % 3) + 2; size > 0; size--) {
    if (Get() % 2 == 0) {
      cursor_y++;
    } else {
      cursor_x++;
    }

    // Note: This can overflow if you roll the same cursor 5 times in a row.
    // This means that a 5-J or a 4-I is produced instead of a 5-I.
    polyshape |= 1 << (cursor_x*4 + cursor_y);
  }

  // Slight adjustment here due to differences in rotation between WitnessPuzzles and The Witness.
  int rotation = (Get() + 1) % 4;
  for (; rotation > 0; rotation--) {
    polyshape = RotatePolyshape(polyshape);
  }

  return polyshape;
}
