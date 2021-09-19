#include "stdafx.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <numeric>
#include <sstream>
#include <thread>
#include <unordered_map>

Console console;

// Ideas to bring back to the javascript version:
// - Try making a single maskedGrid (and polyGrid?) per puzzle. Then, instead of swapping out the puzzle, just write into that grid.
// - Clean up parameters passed through solveLoop.

unordered_map<int, int> tests = {
  {0x00000000, 0x6F4EE991},
  {0x7E9F198C, 0x7F467745},
  {0x7F467745, 0x6B40944A},
  {0x30B54D65, 0x4EEC3ADE},
  {0x4EEC3ADE, 0x770CED4C},
  {0x39CEB361, 0x3162C2ED},
  {0x4AEB6EEF, 0x26C03356},
  {0x146A66FA, 0x558EB18E},
  {0x02C254EC, 0x2455535E},
  {0x2455535E, 0x5DD452F4},
  {0x4C1961B0, 0x165C90D8},
  {0x165C90D8, 0x17296060},
  {0x136B3ACA, 0x64E0ADBB},
  {0x64E0ADBB, 0x5AAE03BA},
  {0x5B2B502E, 0x789D2AC4},
  {0x789D2AC4, 0x1662E7B9},
  {0x16E085CC, 0x6E701FCF},
  {0x6E701FCF, 0x033887AE},
  {0x66923B80, 0x0A80851C},
  {0x279E0D8B, 0x798B34FE},
  {0x798B34FE, 0x22444E09},
  {0x5BB34364, 0x560D8944},
  {0x560D8944, 0x12AEFB7F},
  {0x73069146, 0x342BC5A9},
  {0x342BC5A9, 0x25BDF501},
  {0x4DC4C570, 0x317E63F3},
  {0x60D3FFE7, 0x7E45C85A},
  {0x1764CD7A, 0x5AEE1095},
  {0x4AF19EE8, 0x3CF7B7C8},
  {0x3CF7B7C8, 0x2B42CABD},
  {0x25D18684, 0x5ED65781},
  {0x5ED65781, 0x4A0B06CB},
  {0x591CD356, 0x7376E0CE},
  {0x7376E0CE, 0x1A12F9B},
  {0x563EB84B, 0x2FB57029},
  {0x2FB57029, 0x30D6AC37},
  {0x15FCAE94, 0x002981D3},
  {0x002981D3, 0x250A43BA},
  {0x3B147EB8, 0x3E8B8055},
  {0x3E8B8055, 0x38966C87},
  {0x5B134979, 0x4F3CD5A5},
  {0x4F3CD5A5, 0x22EE6C47},
  {0x36F5F6EE, 0x4E269F72},
  {0x4E269F72, 0x49AE1D73},
  {0x6C09D1E0, 0x78B20089},
  {0x47E84D1F, 0x641F531A},
  {0x641F531A, 0x44890350},
  {0x48B258F5, 0x34EE621C},
  {0x34EE621C, 0x0E6B376A},
  {0x17ED52D5, 0x5DD92A38},
  {0x5C62F2AA, 0x6429A648},
  {0x6A63F457, 0x483EB252},
  {0x74AD8180, 0x2B113658},
  {0x2B113658, 0x7706E17E},
  {0x3E202585, 0x307F5EA0},
  {0x307F5EA0, 0x7A1D733F},
  {0x568C9CC6, 0x2988B78E},
  {0x2988B78E, 0x4ECAE0EF},
  {0x6D19934F, 0x2A165F7E},
  {0x471CBDBD, 0x2FECE7C4},
  {0x2FECE7C4, 0x6A640D70},
  {0x1603A8AB, 0x4A367DD7},
  {0x4A367DD7, 0x3F7FD451},
  {0x15B7C07F, 0x52BEDCFC},
  {0x52BEDCFC, 0x709E4ED4},
  {0x473B7C70, 0x1266C199},
  {0x1266C199, 0x18302D3F},
  {0x3B744AC0, 0x4FD7A1BE},
  {0x4FD7A1BE, 0x55BBE9E5},
  {0x09FC62B0, 0x18B315EF},
  {0x18B315EF, 0x15650A94},
  {0x394F0AE2, 0x745398D2},
  {0x1C563EA8, 0x622B9620},
  {0x1B8D413A, 0x56B554F7},
  {0x47338A38, 0x08B7870D},
};

vector<tuple<int, int, vector<int>>> tests2 = {
  {0x323CE9B1, 0x1621C93B, {0, 1, 2, 3}},
  {0x699ECDDF, 0x54260019, {2, 3, 0, 1}},
  {0x2CFE7867, 0x7039AE2E, {1, 2, 3, 0}},
  {0x7039AE2E, 0x7999D9A3, {0, 3, 2, 1}},
  {0x54647333, 0x3FC55A34, {3, 0, 1, 2}},
};

int main(int argc, char* argv[]) {
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

  if (argc > 1 && strcmp(argv[1], "test") == 0) {
    Random rng;
    for (const auto [key, value] : tests) {
      rng.Set(key);
      assert(rng.Get() == value);
    }
    for (const auto [initRng, endRng, shuffled] : tests2) {
      rng.Set(initRng);
      vector<int> test = {0, 1, 2, 3};
      rng.ShuffleIntegers(test);
      assert(rng.Peek() == endRng);
      assert(test == shuffled);
    }
    rng.GeneratePolyominos(true);
    rng.GeneratePolyominos(true);
    rng.GeneratePolyominos(true);

    cout << "Done" << endl;

  } else if (argc > 1 && strcmp(argv[1], "rand") == 0) {
    Random rng;
    rng.Set(0x5C64B474);
    Puzzle* p = rng.GeneratePolyominos(true);
    cout << *p << endl;
    auto solutions = Solver(p).Solve();
    delete p;

  } else if (argc > 1 && strcmp(argv[1], "period") == 0) {
    const int numThreads = 8;
    vector<thread> threads;
    string results[8];
    int maxValues[8];
    int minValues[8];
    long numSteps[8];
    for (int i=0; i<numThreads; i++) {
      thread t([&](int i) {
        Random rng;
        rng.Set(111111 + i);
        long count = 0;
        int value = 0;
        int maxValue = -0x7FFFFFFF;
        int minValue = 0x7FFFFFFF;
        do {
          value = rng.Get();
          count++;
          maxValue = max(value, maxValue);
          minValue = min(value, minValue);
        } while (!(value >= 111111 && value < 111111 + numThreads));
        stringstream result;
        result << "From " << 111111 + i << " to " << value << " in " << count << " steps";
        results[i] = result.str();
        maxValues[i] = maxValue;
        minValues[i] = minValue;
        numSteps[i] = count;
      }, i);
      threads.push_back(std::move(t));
    }
    for (auto& thread : threads) {
      if (thread.joinable()) thread.join();
    }
    cout << "The complete loop goes:" << endl;
    cout << results[0] << endl;
    cout << results[3] << endl;
    cout << results[6] << endl;
    cout << results[5] << endl;
    cout << results[7] << endl;
    cout << results[1] << endl;
    cout << results[2] << endl;
    cout << results[4] << endl;
    cout << "(a total of " << accumulate(&numSteps[0], &numSteps[numThreads], 0) << " steps)" << endl;
    cout << "The values ranged from 0x" << hex << uppercase << setfill('0') << setw(8) << *min_element(&minValues[0], &minValues[numThreads]);
    cout << " to 0x" << hex << uppercase << setfill('0') << setw(8) << *max_element(&maxValues[0], &maxValues[numThreads]) << endl;

  } else if (argc > 1 && strcmp(argv[1], "thrd") == 0) {
    vector<thread> threads;
    #if _DEBUG
    const int numThreads = 1;
    const int maxSeed = 0x10;
    #else
    const int numThreads = 4;
    const int maxSeed = 0x10'0000; // Maximum of 0x7FFF'FFFE;
    #endif
    int** data = NewDoubleArray<int>(numThreads, maxSeed / numThreads);
    for (int i=0; i<numThreads; i++) {
      thread t([&](int i) {
        Random rng;
        for (int j=0; j<maxSeed / numThreads; j++) {
          int seed = 1 + i + (j * numThreads); // RNG starts at 1
          rng.Set(seed);
          Puzzle* p = rng.GeneratePolyominos(false, true);
          if (!p) continue; // If stars fail, then we will hit this seed in another thread, and there's no reason to solve.
          bool solvable = Solver(p).IsSolvable();
          delete p;
          if (!solvable) continue;

          // Write all valid ending RNGs as contiguous bytes into the file. This is not designed to be human-readable.
          int endingRng = rng.Peek();
          data[i][j] = endingRng;
        }
      }, i);
      threads.push_back(std::move(t));
    }

    for (auto& thread : threads) {
      if (thread.joinable()) thread.join();
    }
    return 0; /*
    // 0b0111 1111 1111 1111 ' 1111 1111 1111 1111
    // We know that the maximum seed value is 0x7FFF'FFFE
    // So we shift right by 7, which gets a maximum of 0xFF'FFFF (which is approximately the number of seeds)
    // Then we use the bottom byte for the bucket, and the remaining 2 bytes for the index.
    // FOR NOW, we are just counting the number of input RNGs which generate each output RNG. This may change.
    short** data2 = NewDoubleArray<short>(0xFF, 0xFFFF);

    // ... I'm not sure what the hell to do here.

    Random rng;
    for (int i=1; i<maxSeed; i++) {
      rng.Set(i);
      rng.GeneratePolyominos(false);
      // ...
    }*/
  }

  return 0;
}