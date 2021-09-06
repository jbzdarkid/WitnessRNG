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

void Random::ShuffleIntegers(vector<int>& arr) {
  for (size_t size = arr.size(); size > 1; size--) {
    size_t rng = Get() % size;
    int tmp = arr[rng];
    arr[rng] = arr[size-1];
    arr[size-1] = tmp;
  }
}
