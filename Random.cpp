#include "stdafx.h"

constexpr int m_prime = 0x7FFFFFFF; // 2^31 - 1, a mersenne prime

int Random::Get() {
  if (_seed == 0) {
    _seed = 111111;
  }
  // This first operation is relatively low entropy, since the multiplicand is close to -2^31 (-2'147'483'648)
  // Thus, this operation is approximately (int32_t)((float)this._current_rng / -2.053f);
  assert(((int)(_seed * 0xFFFF'FFFF'834E'0B5FL / 0x1'0000'0000L)) == ((int)((_seed * 0xFFFF'FFFF'834E'0B5FL) >> 32)));
  int nextRng = (int)((_seed * 0xFFFF'FFFF'834E'0B5FL) >> 32);
  assert(((_seed + nextRng) / 0x1'0000) == ((_seed + nextRng) >> 16));
  nextRng = (_seed + nextRng) >> 16;
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

int Random::Peek() {
  return _seed;
}

void Random::Set(int seed) {
  _seed = seed;
}

void Random::ShuffleInt(Vector<int>& arr) {
  int size = arr.Size();
  for (int i = 0; i < size; i++) {
    int rng1 = Get() % size;
    int rng2 = Get() % size;
    int tmp = arr[rng1];
    arr[rng1] = arr[rng2];
    arr[rng2] = tmp;
  }
}

void Random::ShuffleIntegers(Vector<int>& arr) {
  for (int size = arr.Size(); size > 1; size--) {
    int rng = Get() % size;
    int tmp = arr[rng];
    arr[rng] = arr[size - 1];
    arr[size - 1] = tmp;
  }
}

u16 Random::RandomPolyshape() {
  u8 cursorX = 0;
  u8 cursorY = 0;
  u16 polyshape = 1;

  for (u8 size = (Get() % 3) + 2; size > 0; size--) {
    if (Get() % 2 == 0) {
      cursorY++;
    } else {
      cursorX++;
    }

    // Note: This can overflow if you roll the same cursor 5 times in a row.
    // This means that a 5-J or a 4-I is produced instead of a 5-I.
    polyshape |= 1 << (cursorX * 4 + cursorY);
  }

  // Slight adjustment here due to differences in rotation between WitnessPuzzles and The Witness.
  u8 rotation = (Get() + 1) % 4;
  for (; rotation > 0; rotation--) {
    polyshape = Polyominos::RotatePolyshape(polyshape);
  }

  return polyshape;
}

int Random::CheckStarsFailure() {
  int rngBefore = _seed;
  for (int k = 0; k < 11; k++) Get(); // Initial color generation

  int i = 0;

  // hacky, stolen from puzzle
  while (true) {
    int rand = Get() % (4 * 4); i++;
    int x1 = (rand % 4) * 2 + 1;
    int y1 = (4 - rand / 4) * 2 - 1;

    int x2, y2;
    do {
      rand = Get() % (4 * 4); i++;
      x2 = (rand % 4) * 2 + 1;
      y2 = (4 - rand / 4) * 2 - 1;
    } while (x1 == x2 && y1 == y2);

    if (abs(x1 - x2) + abs(y1 - y2) >= 6) break;
  }

  _seed = rngBefore;
  return i;
}

Random::Random() {
  _visitOrder = new Vector<int>({0, 1, 2, 3});
  _puzzleOrder = new Vector<int>({0, 1, 2, 3});
}

Random::~Random() {
  delete _visitOrder;
  delete _puzzleOrder;
}

Vector<Puzzle*> Random::GenerateChallenge() {
  u8 triple2 = (Get() >> 10) % 3;
  u8 triple3 = (Get() >> 10) % 3;

  for (u8 i=0; i<4; i++) _visitOrder->At(i) = i;
  for (u8 i=0; i<4; i++) _puzzleOrder->At(i) = i;
  ShuffleIntegers(*_visitOrder);
  ShuffleIntegers(*_puzzleOrder);

  Vector<Puzzle*> challenge(18); // 3 + 1 + 4 + 6 + 2 + 2
  challenge.UnsafePush(GenerateSimpleMaze());
  challenge.UnsafePush(GenerateHardMaze());
  challenge.UnsafePush(GenerateStones());

  challenge.UnsafePush(GeneratePedestal());

  // Note that we don't actually care about the visit order (aka locations), but they would apply here too.
  for (u8 i = 0; i < 4; i++) {
    if (_puzzleOrder->At(i) == 0) challenge.UnsafePush(GeneratePolyominos(true));
    if (_puzzleOrder->At(i) == 1) challenge.UnsafePush(GenerateStars());
    if (_puzzleOrder->At(i) == 2) challenge.UnsafePush(GenerateSymmetry());
    if (_puzzleOrder->At(i) == 3) challenge.UnsafePush(GenerateHardMaze());
  }

  challenge.UnsafePush(GenerateTriple2(triple2 == 0));
  challenge.UnsafePush(GenerateTriple2(triple2 == 1));
  challenge.UnsafePush(GenerateTriple2(triple2 == 2));

  challenge.UnsafePush(GenerateTriple3(triple3 == 0));
  challenge.UnsafePush(GenerateTriple3(triple3 == 1));
  challenge.UnsafePush(GenerateTriple3(triple3 == 2));

  challenge.UnsafePush(GenerateTriangles(6));
  challenge.UnsafePush(GenerateTriangles(8));

  challenge.UnsafePush(GenerateDotsPillar());
  challenge.UnsafePush(GenerateStonesPillar());

  return challenge;
}

Puzzle* Random::GenerateSimpleMaze() {
  Puzzle* p = new Puzzle(3, 3);
  p->_name = "Random easy maze #" + std::to_string(_seed);

  do {
    p->ClearGrid();
    p->SetStart(0, 6);
    p->SetEnd(6, 0, End::Top);

    p->CutRandomEdges(*this, 9);
  } while (!IsSolvable(p));

  return p;
}

Puzzle* Random::GenerateHardMaze() {
  Puzzle* p = new Puzzle(7, 7);
  p->_name = "Random hard maze #" + std::to_string(_seed);

  do {
    p->ClearGrid();
    p->SetStart(0, 14);
    p->SetEnd(14, 0, End::Top);

    p->CutRandomEdges(*this, 57);
  } while (!IsSolvable(p));

  return p;
}

Puzzle* Random::GenerateStones() {
  Puzzle* p = new Puzzle(4, 4);
  p->_name = "Random stones #" + std::to_string(_seed);

  do {
    p->ClearGrid();
    p->SetStart(0, 8);
    p->SetEnd(8, 0, End::Top);

    for (int i = 0; i < 7; i++) {
      Cell* cell = p->GetRandomCell(*this);
      cell->type = Type::Square;
      cell->color = 0x2; // White
    }

    for (int i = 0; i < 4; i++) {
      Cell* cell = p->GetRandomCell(*this);
      cell->type = Type::Square;
      cell->color = 0x1; // Black
    }

    if (p->TestStonesEarlyFail()) continue;

    p->CutRandomEdges(*this, 5);
  } while (!IsSolvable(p));

  return p;
}

Puzzle* Random::GeneratePedestal() {
  Puzzle* p = new Puzzle(5, 5);
  p->_name = "Random pedastal #" + std::to_string(_seed);

  do {
    p->ClearGrid();
    p->SetStart(0, 10);
    p->SetEnd(10, 0, End::Right);

    p->CutRandomEdges(*this, 25);
    p->AddRandomDots(*this, 2);
  } while (!IsSolvable(p));

  return p;
}

Puzzle* Random::GeneratePolyominos(bool rerollOnImpossible) {
  Puzzle* p = new Puzzle(4, 4);
  p->_name = "Random polyominos #" + std::to_string(_seed);

  // This only happens once per random generation -- an invalid puzzle will still use the same colors
  Get();
  Vector<int> colors = {0xFFA800, 0x7F00FF, 0x69A957, 0xFF2DFF, 0x2DFFFF};
  ShuffleInt(colors);

  rerollPuzzle:
  {
    p->SetStart(0, 8);
    p->SetEnd(8, 0, End::Right);

    rerollStars:
    Cell* star1 = p->GetEmptyCell(*this);
    Cell* star2 = p->GetEmptyCell(*this);

    // Manhattan Distance of 3 or more
    if (abs(star1->x - star2->x) + abs(star1->y - star2->y) < 6) goto rerollStars;

    star1->type = Type::Star;
    star1->color = colors[0];
    star2->type = Type::Star;
    star2->color = colors[0];

    p->CutRandomEdges(*this, 8);

    u16 polyshape1 = RandomPolyshape();
    u16 polyshape2 = RandomPolyshape();
    Cell* poly1 = p->GetEmptyCell(*this);
    poly1->type = Type::Poly;
    poly1->color = colors[1];
    poly1->polyshape = polyshape1;

    Cell* poly2 = p->GetEmptyCell(*this);
    poly2->type = Type::Poly;
    poly2->color = colors[1];
    poly2->polyshape = polyshape2;

    if (rerollOnImpossible) {
      if (!IsSolvable(p)) {
        if (_seed == 0x7db993b5) { // This seed is known to be solvable (and is used by the tests)
          auto solutions = Solver().Solve(p);
          __debugbreak();
        }
        p->ClearGrid();
        goto rerollPuzzle;
      }
    }
  }

  return p;
}

Puzzle* Random::GenerateStars() {
  Puzzle* p = new Puzzle(4, 4);
  p->_name = "Random stars #" + std::to_string(_seed);

  do {
    p->ClearGrid();
    p->SetStart(0, 8);
    p->SetEnd(8, 0, End::Right);

    p->CutRandomEdges(*this, 10);

    for (int i = 0; i < 4; i++) {
      Cell* cell = p->GetEmptyCell(*this);
      cell->type = Type::Star;
      cell->color = 0x5; // Green
    }

    // This uses 'get_empty_dot_spot' instead of 'add_exactly_this_many_bisection_dots', may matter.
    p->AddRandomDots(*this, 4);
  } while (!IsSolvable(p));

  return p;
}

Puzzle* Random::GenerateSymmetry() {
  Puzzle* p = new Puzzle(6, 6);
  p->_name = "Random symmetry #" + std::to_string(_seed);
  p->_symmetry = SYM_XY;

  do {
    p->ClearGrid();
    p->SetStart(0, 12);
    p->SetStart(12, 0);
    p->SetEnd(12, 12, End::Right);
    p->SetEnd(0, 0, End::Left);

    p->CutRandomEdges(*this, 6);

    // This uses 'get_empty_dot_spot' instead of 'add_exactly_this_many_bisection_dots', may matter.
    p->AddRandomDots(*this, 2, Dot::Blue);
    p->AddRandomDots(*this, 2, Dot::Yellow);
    p->AddRandomDots(*this, 2, Dot::Black);
  } while (!IsSolvable(p));

  return p;
}

Puzzle* Random::GenerateTriple2(bool shouldBeSolvable) {
  Puzzle* p = new Puzzle(4, 4);
  p->_name = "Random triple2 #" + std::to_string(_seed) + (shouldBeSolvable ? " (solvable)" : " (unsolvable)");

  do {
    p->ClearGrid();
    p->SetStart(0, 8);
    p->SetEnd(8, 0, End::Right);

    for (int i = 0; i < 6; i++) {
      Cell* cell = p->GetEmptyCell(*this);
      cell->type = Type::Square;
      cell->color = 0x2; // White
    }

    for (int i = 0; i < 6; i++) {
      Cell* cell = p->GetEmptyCell(*this);
      cell->type = Type::Square;
      cell->color = 0x1; // Black
    }
  } while (!IsSolvable(p));
  // } while (IsSolvable(p) != shouldBeSolvable);

  return p;
}

Puzzle* Random::GenerateTriple3(bool shouldBeSolvable) {
  Puzzle* p = new Puzzle(4, 4);
  p->_name = "Random triple3 #" + std::to_string(_seed) + (shouldBeSolvable ? " (solvable)" : " (unsolvable)");

  do {
    p->ClearGrid();
    p->SetStart(0, 8);
    p->SetEnd(8, 0, End::Right);

    for (int i = 0; i < 5; i++) {
      Cell* cell = p->GetEmptyCell(*this);
      cell->type = Type::Square;
      cell->color = 0x2; // White
    }

    for (int i = 0; i < 2; i++) {
      Cell* cell = p->GetEmptyCell(*this);
      cell->type = Type::Square;
      cell->color = 0x4; // Purple
    }

    for (int i = 0; i < 2; i++) {
      Cell* cell = p->GetEmptyCell(*this);
      cell->type = Type::Square;
      cell->color = 0x5; // Green
    }

    // TW does not allow for L shapes of 3 different colors, in both solvable and non-solvable triples.
    if (p->TestStonesEarlyFail()) continue;
  } while (!IsSolvable(p));
  // } while (IsSolvable(p) != shouldBeSolvable);

  return p;
}

Puzzle* Random::GenerateTriangles(u8 count) {
  Puzzle* p = new Puzzle(4, 4);
  p->_name = "Random " + std::to_string(count) + " triangle #" + std::to_string(_seed);

  do {
    p->ClearGrid();
    p->SetStart(0, 8);
    p->SetEnd(8, 0, End::Right);

    for (u8 i = 0; i < count; i++) {
      Cell* cell = p->GetEmptyCell(*this);
      cell->type = Type::Triangle;
      u8 rng = Get() % 100;
      if (rng > 85)       cell->count = 3;
      else if (rng > 50)  cell->count = 2;
      else                cell->count = 1;
    }
  } while (!IsSolvable(p));

  return p;
}

Puzzle* Random::GenerateDotsPillar() {
  Puzzle* p = new Puzzle(6, 6, true);
  p->_name = "Random dots pillar #" + std::to_string(_seed);
  if (Get() & 0x01) p->_symmetry |= SYM_X; // Horizontal symmetry
  if (Get() & 0x01) p->_symmetry |= SYM_Y; // Vertical symmetry

  do {
    p->ClearGrid();
    p->SetStart(0, 0);
    p->SetEnd(0, 12, End::Top);

    for (u8 i = 0; i < 8; i++) {
      if ((Get() & 0x01) != 0) {
        // u8 v62 = Get() % 7;
        // u8 v63 = Get() % 6;
        // u8 v64 = 6 * (v63 + 6 * v62) + 2;
        // u8 v65 = 6 * (v63 + 6 * v62) + 3;
        // bisect_for_touch_me(this, vertex_map[v64], vertex_map[v65], 0, 0i64, 0i64);
      } else {
        // u8 v66 = Get() % 6;
        // u8 v67 = Get() % 6;
        // u8 v64 = 6 * (v67 + 6 * v66);
        // u8 v65 = 6 * (v67 + 6 * v66) + 36;
        // bisect_for_touch_me(this, vertex_map[v64], vertex_map[v65], 0, 0i64, 0i64);
      }
    }
  } while (!IsSolvable(p));

  return p;
}

Puzzle* Random::GenerateStonesPillar() {
  Puzzle* p = new Puzzle(6, 6, true);
  p->_name = "Random stones pillar #" + std::to_string(_seed);
  if (Get() & 0x01) p->_symmetry |= SYM_X; // Horizontal symmetry
  if (Get() & 0x01) p->_symmetry |= SYM_Y; // Vertical symmetry

  do {
    p->ClearGrid();
    p->SetStart(0, 0);
    p->SetEnd(0, 12, End::Top);

    Cell* cell;
    for (u8 i = 0; i < 3; i++) {
      cell = p->GetEmptyCell(*this);
      cell->type = Type::Square;
      cell->color = 0x2; // White

      cell = p->GetEmptyCell(*this);
      cell->type = Type::Square;
      cell->color = 0x1; // Black
    }

  } while (!IsSolvable(p));

  return p;
}

#include "Windows.h"
#include <mutex>

static Vector<u16> solvability;
static std::mutex solvabilityLock;

bool Random::IsSolvable(int seed) {
  if (solvability.Empty()) {
    std::lock_guard<std::mutex> lock(solvabilityLock);
    if (solvability.Empty()) {
      solvability.Expand(1 << 27);
      solvability.Resize(1 << 27);
      HANDLE file = CreateFileA("puzzle_solvability.dat", FILE_GENERIC_READ, NULL, nullptr, OPEN_EXISTING, NULL, nullptr);
      assert(file != INVALID_HANDLE_VALUE);
      ReadFile((HANDLE)file, &solvability[0], solvability.Size() * sizeof(solvability[0]), nullptr, nullptr);
      CloseHandle(file);
    }
  }

  return (solvability[seed >> 4] & (1 << (seed % 16))) != 0;
}

static Solver solver;

bool Random::IsSolvable(Puzzle* p) {
  return !solver.Solve(p, 1).Empty();
}