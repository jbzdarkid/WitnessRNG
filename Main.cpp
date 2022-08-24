#include "stdafx.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <numeric>
#include <sstream>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#include "File.h"
#include <mutex>

using namespace std;

Console console;

// Functions I wish std::string had
bool Contains(const std::string_view& str, const std::string_view& substr) {
  if (substr.size() > str.size()) return false;
  if (substr.size() == str.size()) return substr == str;
  for (int i = 0; i <= str.size() - substr.size(); i++) {
    if (str.compare(i, substr.size(), substr) == 0) return true;
  }
  return false;
}

// Ideas to bring back to the javascript version:
// - Try making a single maskedGrid (and polyGrid?) per puzzle. Then, instead of swapping out the puzzle, just write into that grid.
// - Clean up parameters passed through solveLoop.
// - Region should be [cells] not [(x, y)]. (uh, maybe not, unsure of if we store x,y on cell)
// - Use RegionData to return "info about the puzzle state", rather than members on the puzzle.

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

vector<tuple<int, int, int>> tests3 = {
  { 0x00000001, 0x618FB492, 0 },
  { 0x00000009, 0x4FF76178, 0 },
  { 0x00000011, 0x6DF2FFE9, 0 },
  { 0x00000019, 0x337647A4, 0 },
  { 0x00000229, 0x34265078, 2 },
  { 0x00000319, 0x7134A0C2, 3 },
  { 0x00001031, 0x23D1E006, 1 },
  { 0x00002351, 0x6B33B70C, 1 },
};

string PrintPolyish(u64 polyish, u8 width, u8 height, u32 polyKey) {
#define IS_SET(grid, x, y) ((grid & (1ull << (x * height + y))) != 0)

  while ((polyish & 0x0101'0101'0101'0101) == 0) polyish >>= 1;
  while ((polyish & 0x0000'0000'0000'00FF) == 0) polyish >>= 8;

  u16 polyshape2 = polyKey & 0xFFFF;
  u16 polyshape1 = Polyominos::Normalize(polyKey >> 16);
  // Unpack the polyshape into the original polyominos.
  u64 shiftedShape = 0;
  for (u8 x=0; x<width; x++) {
    for (u8 y=0; y<height; y++) {
      shiftedShape = (polyshape2 & 0xF000) << 12 | (polyshape2 & 0x0F00) << 8 | (polyshape2 & 0x00F0) << 4 | (polyshape2 & 0x000F);
      shiftedShape <<= (u16)x * 8;
      shiftedShape <<= y;
      if ((polyish & shiftedShape) == shiftedShape) {
        // Found a possible placement for the first one, but it might not agree with the second
        u64 test = polyish & ~shiftedShape;
        while ((test & 0x0101'0101'0101'0101) == 0) test >>= 1;
        while ((test & 0x0000'0000'0000'00FF) == 0) test >>= 8;
        u64 poly1 = (polyshape1 & 0xF000) << 12 | (polyshape1 & 0x0F00) << 8 | (polyshape1 & 0x00F0) << 4 | (polyshape1 & 0x000F);
        if (test == poly1) goto found;
      }
    }
  }

  found: assert(shiftedShape != 0);

  u8 maxX = 0x00;
  u8 maxY = 0x00;
  for (u8 x=0; x<width; x++) {
    for (u8 y=0; y<height; y++) {
      if (IS_SET(polyish, x, y)) {
        maxX = max(maxX, x);
        maxY = max(maxY, y);
      }
    }
  }

  string output;
  for (u8 y = 0; y <= maxY; y++) {
    for (u8 x = 0; x <= maxX; x++) {
      if (IS_SET(polyish, x, y)) {
        if (IS_SET(shiftedShape, x, y)) {
          output += '#';
        } else {
          output += '@';
        }
      } else {
        output += ' ';
      }
    }
    output += '\n';
  }
  return output;
};


string PrintPolykey(u32 polykey, u8 width=8, u8 height=4) {
#define IS_SET(grid, x, y) ((grid & (1ull << (x * height + y))) != 0)

  u8 minX = 0xFF;
  u8 minY = 0xFF;
  u8 maxX = 0x00;
  u8 maxY = 0x00;
  for (u8 x=0; x<width; x++) {
    for (u8 y=0; y<height; y++) {
      if (IS_SET(polykey, x, y)) {
        minX = min(minX, x);
        minY = min(minY, y);
        maxX = max(maxX, x);
        maxY = max(maxY, y);
      }
    }
  }

  string output;
  for (u8 y = minY; y <= maxY; y++) {
    for (u8 x = minX; x <= maxX; x++) {
      if (IS_SET(polykey, x, y)) {
        if (x >= 4) {
          output += '@';
        } else {
          output += '#';
        }
      } else {
        output += ' ';
      }
    }
    output += '\n';
  }
  return output;
};

string GenerateTableCell(u16 polyshape, bool isColumn) {
  u32 shiftedShape = (u32)polyshape << (isColumn ? 16 : 0);
  return "<td class='polykey'>" + PrintPolykey(shiftedShape) + "</td>";
}

void GenerateTable(const unordered_map<u32, u32>& totalPuzzles, u64 uberTotal, const vector<u16>& poly1s, const vector<u16>& poly2s) {
  u64 tableTotal = 0;

  string tableType = to_string(__popcnt(poly1s[0])) + "x" + to_string(__popcnt(poly2s[0]));
  stringstream table;
  table << "<table cellspacing='0px'><tr><td>" << tableType << "</td>";

  for (u16 poly2 : poly2s) table << GenerateTableCell(poly2, false);
  for (u16 poly1 : poly1s) {
    table << "</tr><tr>" << GenerateTableCell(poly1, true);

    for (int i=0; i<poly2s.size(); i++) {
      u16 poly2 = poly2s[i];
      // Adjustment, see explanation in "good".
      u32 polykey = (poly1 << (poly2 & 0xF000 ? 20 : 16)) | poly2;
      auto search = totalPuzzles.find(polykey);
      if (search == totalPuzzles.end()) {
        table << "<td></td>";
      } else {
        u64 total = search->second;
        table << "<td><a href='#" << polykey << "'>" << fixed << setprecision(2) << 100.0 * total / uberTotal << "%</a></td>";
        tableTotal += total;
      }
    }
  }
  table << "</tr></table><br>";

  cout << tableType << " puzzles comprise " << setprecision(2) << (100.0f * tableTotal) / uberTotal << "% (" << tableTotal << ") of all puzzles\n";
  cout << table.str();
}

void GenerateTables(const unordered_map<u32, u32>& totalPuzzles, u64 uberTotal) {
  GenerateTable(totalPuzzles, uberTotal, { 0x0007, 0x0013 }, { 0x000F, 0x0017, 0x0113, 0x1111, 0x0036, 0x0074, 0x0047, 0x0132, 0x0071, 0x0063 });
  GenerateTable(totalPuzzles, uberTotal, { 0x0007, 0x0013 }, { 0x0013, 0x0111, 0x0007, 0x0032, 0x0023});
  GenerateTable(totalPuzzles, uberTotal, { 0x0007, 0x0013 }, { 0x001F, 0x0117, 0x1113, 0x0174, 0x0136, 0x003E, 0x008F, 0x00F8, 0x0364, 0x00F1, 0x0447, 0x1132, 0x0744, 0x0326, 0x007C, 0x00C7, 0x0471, 0x00E3, 0x0463 });
  GenerateTable(totalPuzzles, uberTotal, { 0x000F, 0x0017, 0x0036 }, { 0x0017, 0x0113, 0x0036, 0x000F, 0x0132, 0x0074, 0x0223, 0x0071, 0x0322, 0x0047, 0x0063, 0x0231 });
  GenerateTable(totalPuzzles, uberTotal, { 0x000F, 0x0036, 0x0017 }, { 0x001F, 0x0117, 0x0136, 0x003E, 0x0174, 0x008F, 0x1113, 0x0744, 0x0326, 0x3111, 0x0711, 0x00F1, 0x3222, 0x007C, 0x0447, 0x1132, 0x2223, 0x1322, 0x00F8, 0x2311, 0x0364, 0x0463, 0x0623, 0x00E3, 0x00C7, 0x0471, 0x0631 });
  GenerateTable(totalPuzzles, uberTotal, { 0x001F, 0x0117, 0x003E, 0x0136 }, { 0x0174, 0x003E, 0x0447, 0x0136, 0x0117, 0x0744, 0x0364, 0x007C, 0x0326, 0x00F1, 0x0463, 0x0471, 0x00F8, 0x0623, 0x001F, 0x008F, 0x00E3, 0x00C7, 0x0711, 0x0631 });
}

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
    for (const auto& [initRng, endRng, shuffled] : tests2) {
      rng.Set(initRng);
      Vector<int> test = {0, 1, 2, 3};
      rng.ShuffleIntegers(test);
      assert(rng.Peek() == endRng);
      for (int i=0; i<4; i++) assert(test[i] == shuffled[i]);
    }

    /* Cannot run solvability checks because I don't have the file :(
    for (const auto& [initRng, endRng, numSolutions] : tests3) {
      rng.Set(initRng);
      Puzzle* p = rng.GeneratePolyominos(false);
      assert(rng.Peek() == endRng);
      auto solutions = Solver(p).Solve();
      assert(solutions.Size() == numSolutions);

      // See comment in merge. We need to increment until we reach the loop start.
      rng.Set(initRng);
      int extra = rng.CheckStarsFailure();

      for (int k=0; k<extra - 2; k++) rng.Get();
      // rng is now set to the initial seed that will *actually* generate this result.
      assert(rng.CheckStarsFailure() == 2);

      for (int k=0; k<13; k++) rng.Get();
      assert((numSolutions > 0) == Random::IsSolvable(rng.Peek()));
      delete p;
    }
    */
    cout << "Done" << endl;

  } else if (argc > 1 && strcmp(argv[1], "period") == 0) {
    const int numThreads = 8;
    vector<thread> threads;
    string results[8];
    s64 maxValues[8];
    s64 minValues[8];
    long numSteps[8];
    for (int i=0; i<numThreads; i++) {
      thread t([&](int i) {
        Random rng;
        rng.Set(111111 + i);
        long count = 0;
        int value = 0;
        s64 maxValue = -0x7FFF'FFFF'FFFF'FFFF;
        s64 minValue = 0x7FFF'FFFF'FFFF'FFFF;
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
      threads.push_back(move(t));
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

  } else if (argc > 1 && strcmp(argv[1], "seed") == 0) {
    Random rng;
    rng.Set(0x48); // ... somehow, this is 10? ... and 0x2A is 1. I think there was a bug in my string parser, and sadly I don't have source code right now.
    Vector<Puzzle*> challenge = rng.GenerateChallenge();
    // For seed 10, we expect:
    // Stars, top right
    assert(Contains(challenge[4]->_name, "stars"));
    // Symmetry, top left
    assert(Contains(challenge[5]->_name, "symmetry"));
    // Polys, bottom left
    assert(Contains(challenge[6]->_name, "poly"));
    // Maze, back
    assert(Contains(challenge[7]->_name, "maze"));
    // Right b/w
    assert(Contains(challenge[10]->_name, "(solvable)"));
    // Center 3col
    assert(Contains(challenge[12]->_name, "(solvable)"));

    for (auto puzzle : challenge) {
      cout << puzzle->ToString() << endl;
      delete puzzle;
    }

  } else if (argc > 1 && strcmp(argv[1], "rseed") == 0) {
    string line;

    string easy;
    cout << "Input the first puzzle (easy maze), in a 9x9 grid. Empty line finishes." << endl;
    while (true) {
      getline(cin, line);
      if (line.empty()) break;
      easy += line;
    }

    string stones;
    cout << "[OPTIONAL] Input the third puzzle (stones), in a 4x4 grid, e.g. 'BW W'. Empty line finishes." << endl;
    while (true) {
      getline(cin, line);
      if (line.empty()) break;
      stones += line;
    }

    cout << "Input the puzzles on the central pillar. For example, 'BL poly', 'TR stars', 'B sym', or 'TL maze'." << endl;
    Vector<int> expectedOrder{0, 0, 0, 0};
    Vector<int> expectedPuzzles{0, 0, 0, 0};
    for (u8 i=0; i<4; i++) {
      cout << to_string(i + 1) << ": ";
      getline(cin, line);
      if      (line.substr(0, 2) == "BL") expectedOrder[i] = 0;
      else if (line.substr(0, 2) == "TR") expectedOrder[i] = 1;
      else if (line.substr(0, 2) == "B ") expectedOrder[i] = 2;
      else if (line.substr(0, 2) == "TL") expectedOrder[i] = 3;
      else { cout << "Couldn't recognize the location, try again" << endl; continue; }

      if      (Contains(line, "poly")) expectedPuzzles[i] = 0;
      else if (Contains(line, "star")) expectedPuzzles[i] = 1;
      else if (Contains(line, "sym"))  expectedPuzzles[i] = 2;
      else if (Contains(line, "maze")) expectedPuzzles[i] = 3;
      else { cout << "Couldn't recognize the puzzle type, try again" << endl; continue; }
    }

    u8 triple2;
    while (true) {
      cout << "Input the b/w triple location (left, middle, right): ";
      getline(cin, line);
      if      (Contains(line, "eft"))   triple2 = 0;
      else if (Contains(line, "iddle")) triple2 = 1;
      else if (Contains(line, "ight"))  triple2 = 2;
      else { cout << "Couldn't recognize the location, try again" << endl; continue; }
      break;
    }

    u8 triple3;
    while (true) {
      cout << "Input the w/g/p triple location (left, middle, right): ";
      getline(cin, line);
      if      (Contains(line, "eft"))   triple3 = 0;
      else if (Contains(line, "iddle")) triple3 = 1;
      else if (Contains(line, "ight"))  triple3 = 2;
      else { cout << "Couldn't recognize the location, try again" << endl; continue; }
      break;
    }

    cout << "Processing..." << endl;

    bool found = false;
    Random rng;
    for (int seed=1; seed <= 0x7FFF'FFFE; seed++) {
      rng.Set(seed);
      if (rng.TestChallenge(triple2, triple3, expectedOrder, expectedPuzzles, easy, stones)) {
        cout << "Found seed: " << seed << endl;
        found = true;
      }
    }
    if (!found) cout << "Couldn't find any matching seeds" << endl;
  } else if (argc > 1 && strcmp(argv[1], "rand") == 0) {
    Random rng;
    rng.Set(806297464);
    // rng.Set(819664878);
    Puzzle* p = rng.GeneratePolyominos(false);
    cout << p->ToString() << endl;
    auto solutions = Solver().Solve(p);
    delete p;

  } else if (argc > 1 && strcmp(argv[1], "thrd") == 0) {
#if _DEBUG
    const int threadOffset = 0;
    const int numThreads = 1;
    const u32 initSeed = 0;
    const u32 maxSeed = 0x4000;
#else
    const int threadOffset = 48;
    const int numThreads = 16;
    const u32 initSeed = 0x6000'0000;
    const u32 maxSeed = 0x7FFF'FFFE;
#endif
    static_assert(maxSeed <= 0x7FFF'FFFE);
    Vector<thread> threads;
    for (u32 i=0; i<numThreads; i++) {
      thread t([&](int i) {
        auto goodFile = CreateFileA(("thread_" + to_string(i+threadOffset) + "_good.dat").c_str(), FILE_GENERIC_WRITE, NULL, nullptr, CREATE_ALWAYS, NULL, nullptr);
        auto badFile  = CreateFileA(("thread_" + to_string(i+threadOffset) + "_bad.dat").c_str(),  FILE_GENERIC_WRITE, NULL, nullptr, CREATE_ALWAYS, NULL, nullptr);

        Random rng;
        Solver solver;
        for (u32 j=0;; j++) {
          u32 seed = initSeed + 1 + i + (j * numThreads); // RNG starts at 1
          if (seed > maxSeed) break;
          rng.Set(seed);

          bool starsFailure = rng.CheckStarsFailure();
          Puzzle* p = rng.GeneratePolyominos(false); // Even if stars fail, we still want to roll the RNG to find the endRng.

          Vector<Path> solutions;
          if (!starsFailure) { // If stars fail, then we will hit this seed in another thread, and there's no reason to solve.
            solutions = solver.Solve(p);
          }

          u32 endingRng = rng.Peek();
          if (solutions.Empty()) {
            WriteFile(badFile, &seed, sizeof(seed), nullptr, nullptr);
            WriteFile(badFile, &endingRng, sizeof(endingRng), nullptr, nullptr);
            SetFilePointer(badFile, 0, nullptr, FILE_END);
          } else {
            WriteFile(goodFile, &seed, sizeof(seed), nullptr, nullptr);
            int numSolutions = solutions.Size();
            WriteFile(goodFile, &numSolutions, sizeof(numSolutions), nullptr, nullptr);
            for (const auto& solution : solutions) {
              WriteFile(goodFile, &solution[0], sizeof(u8) * solution.Size(), nullptr, nullptr);
            }
            // string puzzleStr = p->ToString();
            // WriteFile(goodFile, puzzleStr.c_str(), (DWORD)(sizeof(char) * puzzleStr.size()), nullptr, nullptr);
            SetFilePointer(goodFile, 0, nullptr, FILE_END);
          }
          delete p;
        }
        CloseHandle(badFile);
        CloseHandle(goodFile);
      }, i);
      threads.Emplace(move(t));
    }

    for (int i=0; i<numThreads; i++) {
      if (threads[i].joinable()) threads[i].join();
    }
  } else if (argc > 1 && strcmp(argv[1], "merge") == 0) {
    Vector<u16> finalData(1 << 27); // A single bit per seed
    std::mutex dataLock;
    finalData.Fill((u8)0xFF);

#if _DEBUG
    const int numThreads = 1;
#else
    const int numThreads = 4;
#endif
    Vector<thread> threads(numThreads);

    for (u32 i=0; i<numThreads; i++) {
      thread t([&](int i) {
        Vector<u16> threadData = finalData.Copy();
        Random rng;
        for (int j = i;; j+=numThreads) {
          File badFile("thread_" + to_string(j) + "_bad.dat");
          if (badFile.Done()) break; // File did not exist

          while (!badFile.Done()) {
            u32 initialSeed = badFile.GetInt();
            u32 finalSeed = badFile.GetInt();
            (void)finalSeed;

            // if (initialSeed == 0x000041a7) DebugBreak();
            // if (initialSeed == 1) DebugBreak();

            rng.Set(initialSeed);
            // While generating data, we do not differentiate a generation failure from an unsolvable puzzle, as both incur a reroll.
            // However, for the purposes of computing solvability, we only want to denote actualy unsolvable puzzles,
            // ergo we  need to skip any puzzles which failed due to a stars failure.
            int rerollCount = rng.CheckStarsFailure();
            if (rerollCount > 2) continue;

            // We use a value based on the initial seed here, because the final seed is not actually unique!
            // Two puzzles may share a final seed if they had different polyomino sizes, since differently-sized polyominos
            // will increment the RNG a different number of times. Ergo, differentiating based on the end RNG is not possible.
            // Instead, we use the initial seed. However, if a puzzle fails, we jump to just before the starts failure,
            // which means *that* moment is the "initial" seed for re-rolls. To avoid extra normalization, we always use that location,
            // which is 11 RNG steps beyond the initial seed, +2 for the stars roll.
            for (int k=0; k<13; k++) rng.Get();
            // if (rng.Peek() == 0x6a5d128c) DebugBreak();

            // Computing *solvability* here
            threadData[rng.Peek() >> 4] &= ~(1 << (rng.Peek() % 16));
          }
        }

        dataLock.lock();
        for (int j=0; j<threadData.Size(); j++) {
          finalData[j] &= threadData[j];
        }
        dataLock.unlock();
      }, i);
      threads.Emplace(move(t));
    }
    for (int i=0; i<numThreads; i++) {
      if (threads[i].joinable()) threads[i].join();
    }

    // This file can probably be checked in, honestly. It's only about 256 MB, which is /annoying/ to clone but not that big of a deal. Especially since it'll never change.
    auto output = CreateFileA("puzzle_solvability.dat", FILE_GENERIC_WRITE, NULL, nullptr, CREATE_ALWAYS, NULL, nullptr);
    WriteFile(output, &finalData[0], finalData.Size() * sizeof(finalData[0]), nullptr, nullptr);

  } else if (argc > 1 && strcmp(argv[1], "good") == 0) {
    // using Polykey = u32;
    // using Polyish = u64;
    // using Count = u32;
    // struct Data {
    //   unordered_map<Polyish, Count> puzzlesWhichCanBeSolvedUsingThisPolyish;
    // };
    // unordered_map<Polykey, Data> data;

    unordered_map<u32, unordered_map<u64, u32>> combinedPolyshapes;
    unordered_map<u32, unordered_map<u64, u32>> uniquePolyshapes;
    unordered_map<u32, u32> totalPuzzles;
    unordered_map<u32, u32[4]> starStatistics;
    u64 totalPolysTogether = 0;
    u64 totalPolysApart = 0;
    u64 totalPolysBoth = 0;

    Random rng;
#if _DEBUG
    for (int i = 0; i < 1; i++) {
#else
    for (int i = 0;; i++) {
#endif
      OutputDebugString((L"Starting file: thread_" + to_wstring(i) + L"_good.dat\n").c_str());
      File goodFile("thread_" + to_string(i) + "_good.dat");
      if (goodFile.Done()) break; // File did not exist

      while (!goodFile.Done()) { // Read through the entire file
        u32 seed = goodFile.GetInt();
        rng.Set(seed);
        Puzzle* p = rng.GeneratePolyominos(false);

        Cell* firstPoly = nullptr;
        Cell* secondPoly = nullptr;
        bool canContainStars = false;
        bool canExcludeStars = false;
        std::unordered_set<u64> validPolyshapes;

        // Compute polyshapes
        for (u8 x = 1; x < p->_width; x += 2) {
          for (u8 y = 1; y < p->_height; y += 2) {
            Cell* cell = p->GetCell(x, y);
            u16 polyshape = cell->polyshape;
            if (polyshape == 0) continue;

            if (!firstPoly) {
              firstPoly = cell;
            } else {
              secondPoly = cell;
              break;
            }
          }
        }

        struct NormalizedPolys {
          u16 poly1 = 0xFFFF;
          u16 poly2 = 0xFFFF;
          u8 rotation = 0;
          bool flip = false;
        } min;

        assert(firstPoly);
        assert(secondPoly);
        u16 polyshape1 = firstPoly->polyshape;
        u16 polyshape2 = secondPoly->polyshape;
        bool flipped = false;
        u8 rotation = 0;

        for (u8 j=0; j<8; j++) {
          if (j == 4) {
            polyshape1 = Polyominos::Flip(polyshape1);
            polyshape2 = Polyominos::Flip(polyshape2);
            flipped = !flipped;
            rotation -= 2;
          } else {
            polyshape1 = Polyominos::RotatePolyshape(polyshape1);
            polyshape2 = Polyominos::RotatePolyshape(polyshape2);
            rotation++;
          }
          polyshape1 = Polyominos::Normalize(polyshape1);
          polyshape2 = Polyominos::Normalize(polyshape2);

          for (int k=0; k<2; k++) {
            if (__popcnt16(polyshape1) < __popcnt16(min.poly1)
             || __popcnt16(polyshape1) == __popcnt16(min.poly1) && polyshape1 < min.poly1
             || __popcnt16(polyshape1) == __popcnt16(min.poly1) && polyshape1 == min.poly1 && __popcnt16(polyshape2) < __popcnt16(min.poly2)
             || __popcnt16(polyshape1) == __popcnt16(min.poly1) && polyshape1 == min.poly1 && __popcnt16(polyshape2) == __popcnt16(min.poly2) && polyshape2 < min.poly2) {
              min.poly1 = polyshape1;
              min.poly2 = polyshape2;
              min.rotation = rotation;
              min.flip = flipped;
            }
            swap(polyshape1, polyshape2); // To avoid writing the above logic twice
          }
        }
        u32 polyKey = 0;
        // If the left-hand poly (poly2) fills the right-most column, shift the right-hand poly one row.
        // Due to the minimization, we will never have 4-wide polyominos -- only 4-tall.
        if (min.poly2 & 0xF000) min.poly1 <<= 4;
        polyKey = (min.poly1 << 16) | min.poly2;

        // Compute all solution paths by reading from file
        for(u32 numSolutions = goodFile.GetInt(); numSolutions > 0; numSolutions--) {
          p->ClearGrid(true);
          u8 x = goodFile.Get();
          u8 y = goodFile.Get();
          assert(x == 0 && y == 8);
          while (true) { // Trace the solution path
            Cell* cell = p->GetCell(x, y);
            assert(cell);
            cell->line = Line::Black;
            u8 dir = goodFile.Get();
            if (dir == PATH_NONE) break;
            else if (dir == PATH_LEFT)   x--;
            else if (dir == PATH_RIGHT)  x++;
            else if (dir == PATH_TOP)    y--;
            else if (dir == PATH_BOTTOM) y++;
          }

          Region region = p->GetRegion(firstPoly->x, firstPoly->y);
          bool sameRegion = false;
          bool containsStars = false;
          for (Cell* cell : region) {
            if (cell == secondPoly) sameRegion = true;
            if (cell->type == Type::Star) containsStars = true;
          }

          if (sameRegion) {
            u64 polyish = p->GetPolyishFromMaskedGrid(min.rotation, min.flip);
            assert(__popcnt16(min.poly1) + __popcnt16(min.poly2) == __popcnt64(polyish));
            validPolyshapes.insert(polyish);
          } else {
            validPolyshapes.insert(0);
          }
          if (containsStars) canContainStars = true;
          else               canExcludeStars = true;
        } // done with puzzle

        for (u64 polyish : validPolyshapes) combinedPolyshapes[polyKey][polyish]++;
        if (validPolyshapes.size() == 1) uniquePolyshapes[polyKey][*validPolyshapes.begin()]++;
        totalPuzzles[polyKey]++;

        u8 starsValue = (canContainStars ? 1 : 0) + (canExcludeStars ? 2 : 0);
        starStatistics[polyKey][starsValue]++;

        if (validPolyshapes.find(0) == validPolyshapes.end()) {
          totalPolysTogether++; // No configuration allows the polys to be apart
        } else {
          if (validPolyshapes.size() == 1) {
            totalPolysApart++; // There is only one configuration and it has the polys apart
          } else {
            totalPolysBoth++; // There are multiple configurations allowing apart & together
          }
        }

        delete p;
      } // done reading file
    }

    // Start of file, etc.
    cout << R"(<html>
<head>
  <style>
    * {
      font-family: Constantia;
    }
    pre, .polykey {
      white-space: pre;
      font-family: monospace;
    }
    td {
      border: 1px solid black;
      border-collapse: collapse;
      padding: 2px;
      text-align: center;
      width: 50px;
    }
  </style>
</head>
<body>)";

    u64 uberTotal = 0;
    u64 uberUnique = 0;
    // u64 totalStarsContained = 0;
    // u64 totalStarsExcluded = 0;
    // u64 totalStarsBoth = 0;

    vector<pair<u32, u32>> sortedPolyshapes;
    for (const auto& [polykey, total] : totalPuzzles) {
      uberTotal += total;
      for (const auto& [polyish, count] : uniquePolyshapes[polykey]) uberUnique += count;
      sortedPolyshapes.emplace_back(polykey, total);
    }
    sort(sortedPolyshapes.begin(), sortedPolyshapes.end(), [](const pair<u32, u32>& a, const pair<u32, u32>& b) { return a.second > b.second; });

    cout << "There are a total of " << uberTotal << " polyominos + stars puzzles<br>\n";
    cout << uberUnique << " (" << (100.0f * uberUnique / uberTotal) << "%) of these puzzles can only be solved with one configuration<br>\n";
    cout << totalPolysTogether << " puzzles (" << (100.0f * totalPolysTogether) / uberTotal << "% of all puzzles) must be solved with the polyominos combined<br>\n";
    cout << totalPolysApart << " puzzles (" << (100.0f * totalPolysApart) / uberTotal << "% of all puzzles) must be solved with the polyominos separated<br>\n";
    cout << totalPolysBoth << " puzzles (" << (100.0f * totalPolysBoth) / uberTotal << "% of all puzzles) can be solved either way<br>\n";
    assert(uberTotal == totalPolysTogether + totalPolysApart + totalPolysBoth);
    cout << "<br><span class='anchor' id='table-of-contents'><h2>Table of contents</h2></span>";
    GenerateTables(totalPuzzles, uberTotal);
    cout << fixed; // no scientific notation please

    for (const auto& [polykey, total] : sortedPolyshapes) {
      const auto& combinedData = combinedPolyshapes[polykey];
      const auto& uniqueData = uniquePolyshapes[polykey];
      u32 uniqueTotal = 0;
      for (const auto& [key2, data2] : uniqueData) uniqueTotal += data2;

      cout << "<span class='anchor' id='" << polykey << "'></span>";
      cout << "---------------------------------------------------------------------------------------------------------------------------------------<br>\n";
      cout << "<small><a href='#table-of-contents'>Jump to top</a></small><br>\n";
      cout << "This pair of polyominos is present in " << total << " puzzles (" << (100.0f * total) / uberTotal << "% of all puzzles)<br>\n";
      cout << "Of those puzzles, " << uniqueTotal << " (" << (100.0f * uniqueTotal / total) << "%) can only be solved with one configuration of polyominos<br>\n";
      cout << "<pre>" << PrintPolykey(polykey) << "</pre>";

      vector<pair<u64, u32>> items;
      for (const auto& it : combinedData) items.push_back(it);
      sort(items.begin(), items.end(), [](const pair<u64, u32>& a, const pair<u64, u32>& b) { return a.second > b.second; });

      for (const auto& [polyish, count] : items) {
        u32 uniqueCount = uniqueData.at(polyish);

        if (polyish == 0) {
          cout << count << " (" << (100.0f * count) / total << "%) of these puzzles can be solved with the polyominos separated<br>\n";
          cout << uniqueCount << " (" << (100.0f * uniqueCount) / total << "%) of these puzzles must be solved with the polyominos separated<br>\n";
        } else {
          cout << count << " (" << (100.0f * count) / total << "%) of these puzzles can be solved with the polyominos in this configuration:<br>\n";
          cout << uniqueCount << " (" << (100.0f * uniqueCount) / total << "%) of these puzzles must be solved with the polyominos in this configuration:<br>\n";
          cout << "<pre>" << PrintPolyish(polyish, 8, 8, polykey) << "</pre>";
        }
      }
    }

    cout << "</body></html>";
  }

  return 0;
}
