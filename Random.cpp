#include "stdafx.h"

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

int Random::Peek() {
  return _seed;
}

void Random::Set(int seed) {
  _seed = seed;
}

void Random::ShuffleInt(Vector<int>& arr) {
  int size = arr.Size();
  for (int i=0; i<size; i++) {
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
    arr[rng] = arr[size-1];
    arr[size-1] = tmp;
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
    polyshape |= 1 << (cursorX*4 + cursorY);
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
  for (int k=0; k<11; k++) Get(); // Initial color generation

  int i = 0;

  // hacky, stolen from puzzle
  while (true) {
    int rand = Get() % (4 * 4); i++;
    int x1 = (rand % 4)*2 + 1;
    int y1 = (4 - rand/4)*2 - 1;

    int x2, y2;
    do {
      rand = Get() % (4 * 4); i++;
      x2 = (rand % 4)*2 + 1;
      y2 = (4 - rand/4)*2 - 1;
    } while (x1 == x2 && y1 == y2);

    if (abs(x1 - x2) + abs(y1 - y2) >= 6) break;
  }

  _seed = rngBefore;
  return i;
}

Vector<Puzzle*> Random::GenerateChallenge() {
  Vector<Puzzle*> challenge(18); // 3 + 1 + 4 + 6 + 2 + 2

  u8 triple2 = Get() % 3;
  u8 triple3 = Get() % 3;

  challenge.UnsafePush(GenerateEasyMaze(true));
  challenge.UnsafePush(GenerateHardMaze(true));
  challenge.UnsafePush(GenerateStones(true));
  challenge.UnsafePush(GeneratePedestal(true));

  // Pillar order, somehow
  challenge.UnsafePush(GenerateHardMaze(true));
  challenge.UnsafePush(GenerateSymmetry(true));
  challenge.UnsafePush(GeneratePolyominos(true));
  challenge.UnsafePush(GenerateStars(true));

  challenge.UnsafePush(GenerateTriple2(triple2 == 0));
  challenge.UnsafePush(GenerateTriple2(triple2 == 1));
  challenge.UnsafePush(GenerateTriple2(triple2 == 2));

  challenge.UnsafePush(GenerateTriple3(triple3 == 0));
  challenge.UnsafePush(GenerateTriple3(triple3 == 1));
  challenge.UnsafePush(GenerateTriple3(triple3 == 2));

  challenge.UnsafePush(GenerateTriangles(6, true));
  challenge.UnsafePush(GenerateTriangles(8, true));

  challenge.UnsafePush(GenerateDotsPillar(true));
  challenge.UnsafePush(GenerateStonesPillar(true));

  return challenge;
}

Puzzle* Random::GenerateEasyMaze(bool rerollOnImpossible) {
  Puzzle* p = new Puzzle(3, 3);
  p->_name = "Random easy maze #" + std::to_string(_seed);

  rerollPuzzle:
  {
    p->_grid->Get(0, 6).start = true;
    p->_grid->Get(6, 0).end = End::Right; p->_numConnections++;

    p->CutRandomEdges(*this, 9);

    if (rerollOnImpossible) {
      bool unsolvable = Solver(p).Solve(1).Empty();

      if (unsolvable) {
        p->ClearGrid();
        goto rerollPuzzle;
      }
    }
  }

  return p;
}

Puzzle* Random::GenerateHardMaze(bool rerollOnImpossible) {
  Puzzle* p = new Puzzle(7, 7);
  p->_name = "Random hard maze #" + std::to_string(_seed);

  rerollPuzzle:
  {
    p->_grid->Get(0, 14).start = true;
    p->_grid->Get(14, 0).end = End::Right; p->_numConnections++;

    p->CutRandomEdges(*this, 57);

    if (rerollOnImpossible) {
      bool unsolvable = Solver(p).Solve(1).Empty();

      if (unsolvable) {
        p->ClearGrid();
        goto rerollPuzzle;
      }
    }
  }

  return p;
}

Puzzle* Random::GenerateStones(bool /*rerollOnImpossible*/) {
  return nullptr;
}

Puzzle* Random::GeneratePedestal(bool rerollOnImpossible) {
  Puzzle* p = new Puzzle(5, 5);
  p->_name = "Random pedastal #" + std::to_string(_seed);

  rerollPuzzle:
  {
    p->_grid->Get(0, 10).start = true;
    p->_grid->Get(10, 0).end = End::Right; p->_numConnections++;

    p->CutRandomEdges(*this, 25);
    p->AddRandomDots(*this, 2);

    if (rerollOnImpossible) {
      bool unsolvable = Solver(p).Solve(1).Empty();

      if (unsolvable) {
        p->ClearGrid();
        goto rerollPuzzle;
      }
    }
  }

  return p;
}

Puzzle* Random::GenerateSymmetry(bool /*rerollOnImpossible*/) {
  return nullptr;
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
    p->_grid->Get(0, 8).start = true;
    p->_grid->Get(8, 0).end = End::Right; p->_numConnections++;
    bool solvable = IsSolvable(_seed);
    (void)solvable;

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
      bool unsolvable = Solver(p).Solve(1).Empty();

      if (unsolvable) {
        if (_seed == 0x7db993b5) { // This seed is known to be solvable (and is used by the tests)
          auto solutions = Solver(p).Solve();
          assert(false);
        }
        p->ClearGrid();
        goto rerollPuzzle;
      }
    }
  }

  return p;
}

Puzzle* Random::GenerateStars(bool /*rerollOnImpossible*/) {
  return nullptr;
}

Puzzle* Random::GenerateTriple2(bool /*solvable*/) {
  return nullptr;
}

Puzzle* Random::GenerateTriple3(bool /*solvable*/) {
  return nullptr;
}

Puzzle* Random::GenerateTriangles(u8 count, bool rerollOnImpossible) {
  Puzzle* p = new Puzzle(4, 4);
  p->_name = "Random " + std::to_string(count) + " triangle #" + std::to_string(_seed);

  rerollPuzzle:
  {
    p->_grid->Get(0, 8).start = true;
    p->_grid->Get(8, 0).end = End::Right; p->_numConnections++;

    for (u8 i = 0; i < count; i++) {
      Cell* cell = p->GetEmptyCell(*this);
      cell->type = Type::Triangle;
      u8 rng = Get() % 100;
      if (rng > 0x55)       cell->count = 3;
      else if (rng > 0x32)  cell->count = 2;
      else                  cell->count = 1;
    }

    if (rerollOnImpossible) {
      bool unsolvable = Solver(p).Solve(1).Empty();

      if (unsolvable) {
        p->ClearGrid();
        goto rerollPuzzle;
      }
    }
  }

  return p;
}

Puzzle* Random::GenerateDotsPillar(bool /*rerollOnImpossible*/) {
  return nullptr;
}

Puzzle* Random::GenerateStonesPillar(bool /*rerollOnImpossible*/) {
  return nullptr;
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
      assert(file != INVALID_HANDLE_VALUE)
      ReadFile((HANDLE)file, &solvability[0], solvability.Size() * sizeof(solvability[0]), nullptr, nullptr);
      CloseHandle(file);
    }
  }

  return (solvability[seed >> 4] & (1 << (seed % 16))) != 0;
}