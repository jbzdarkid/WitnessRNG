#include "stdafx.h"

Solver::Solver() {
  path = new Path();
  validator = new Validator();
}

Solver::~Solver() {
  delete path;
  delete validator;
}

Vector<Path> Solver::Solve(Puzzle* puzzle_, int maxSolutions) {
  puzzle = puzzle_;
  path->Ensure(puzzle->_width * puzzle->_height); // A little overkill but whatever.
  path->Resize(0);

  Vector<Cell*> startPoints(puzzle->_width);
  u8 numEndpoints = 0;

  puzzle->_hasNegations = false;
  puzzle->_hasPolyominos = false;
  for (u8 x=0; x<puzzle->_width; x++) {
    for (u8 y=0; y<puzzle->_height; y++) {
      Cell* cell = &puzzle->_grid->Get(x, y);
      if (cell->type == Type::Null) continue;
      if (cell->start == true) {
        startPoints.Push(cell);
      }
      if (cell->end != End::None) numEndpoints++;
      if (cell->type == Type::Nega) puzzle->_hasNegations = true;
      if (cell->type == Type::Poly || cell->type == Type::Ylop) puzzle->_hasPolyominos = true;
    }
  }

  // Some reasonable default data, which will avoid crashes during the solveLoop.
  // var earlyExitData = [false, {"isEdge": false}, {"isEdge": false}]
  if (maxSolutions > 0) MAX_SOLUTIONS = maxSolutions;
  Vector<Path> solutionPaths(MAX_SOLUTIONS);

  // Large pruning optimization -- Attempt to early exit once we cut out a region.
  // Inspired by https://github.com/Overv/TheWitnessSolver
  // For non-pillar puzzles, every time we draw a line from one edge to another, we cut out two regions.
  // We can detect this by asking if we"ve ever left an edge, and determining if we"ve just touched an edge.
  // However, just touching the edge isn"t sufficient, since we could still enter either region.
  // As such, we wait one additional step, to see which half we have moved in to, then we evaluate
  // whichever half you moved away from (since you can no longer re-enter it).
  //
  // Consider this pathway (tracing X-X-X-A-B-C).
  // ....X....
  // . . X . .
  // ....X....
  // . . A . .
  // ...CB....
  //
  // Note that, once we have reached B, the puzzle is divided in half. However, we could go either
  // left or right -- so we don"t know which region is safe to validate.
  // Once we reach C, however, the region to the right is closed off.
  // As such, we can start a flood fill from the cell to the right of A, computed by A+(C-B).
  //
  // Unfortunately, this optimization doesn"t work for pillars, since the two regions are still connected.
  // Additionally, this optimization doesn"t work when custom mechanics are active, as many custom mechanics
  // depend on the path through the entire puzzle
  // doPruning = (puzzle->_pillar == false);
  doPruning = false; // Sigh.

  for (Cell* startPoint : startPoints) {
    // NOTE: This is subtly different from WitnessPuzzles, which starts the path with [[x, y]] instead of [x, y]!
    path->UnsafePush(startPoint->x);
    path->UnsafePush(startPoint->y);
    puzzle->_startPoint = startPoint;
    SolveLoop(startPoint->x, startPoint->y, solutionPaths, numEndpoints);
  }

  return solutionPaths;
}

void Solver::TailRecurse(Cell* cell) {
  cell->line = Line::None;
  if (puzzle->_symmetry != SYM_NONE) {
    Cell* symCell = puzzle->GetSymmetricalCell(cell);
    symCell->line = Line::None;
  }
}

void Solver::SolveLoop(s8 x, s8 y, Vector<Path>& solutionPaths, u8 numEndpoints) {
  // Stop trying to solve once we reach our goal
  if (solutionPaths.Size() >= MAX_SOLUTIONS) return;

  // Check for collisions (outside, gap, self, other)
  Cell* cell = puzzle->GetCell(x, y);
  if (cell == nullptr || cell->type == Type::Null) return;
  if (cell->gap != Gap::None) return;
  if (cell->line != Line::None) return;

  if (puzzle->_symmetry == SYM_NONE) {
    cell->line = Line::Black;
  } else {
    Cell* symCell = puzzle->GetSymmetricalCell(cell);
    if (puzzle->MatchesSymmetricalPos(x, y, symCell->x, symCell->y)) return; // Would collide with our reflection

    if (symCell->gap != Gap::None) return;

    cell->line = Line::Blue;
    symCell->line = Line::Yellow;
  }

  if (cell->end != End::None) {
    path->UnsafePush(PATH_NONE);
    puzzle->_endPoint = cell;
    RegionData puzzleData = validator->Validate(*puzzle, true);
    if (puzzleData.Valid()) {
      solutionPaths.Emplace(path->Copy());
    }
    path->Pop();

    // If there are no further endpoints, tail recurse.
    // Otherwise, keep going -- we might be able to reach another endpoint.
    numEndpoints--;
    if (numEndpoints == 0) {
      TailRecurse(cell);
      return;
    }
  }

  if (doPruning) {
    bool isEdge = x <= 0 || y <= 0 || x >= puzzle->_width - 1 || y >= puzzle->_height - 1;
    EarlyExitData newEarlyExitData = {
      earlyExitData.hasEverLeftEdge || (!isEdge && earlyExitData.isEdge2), // Have we ever left an edge?
      earlyExitData.x2, earlyExitData.y2, earlyExitData.isEdge2,           // The position before our current one
      x, y, isEdge                                                         // Our current position.
    };
    if (earlyExitData.hasEverLeftEdge && !earlyExitData.isEdge1 && earlyExitData.isEdge2 && isEdge) {
      // See the above comment for an explanation of this math.
      s8 floodX = earlyExitData.x2 + (earlyExitData.x1 - x);
      s8 floodY = earlyExitData.y2 + (earlyExitData.y1 - y);
      Region region = puzzle->GetRegion(floodX, floodY);
      if (!region.Empty()) {
        RegionData regionData = validator->ValidateRegion(*puzzle, region, true);
        if (!regionData.Valid()) {
          TailRecurse(cell);
          return;
        }

        // Additionally, we might have left an endpoint in the enclosed region.
        // If so, we should decrement the number of remaining endpoints (and possibly tail recurse).
        for (Cell* endCell : region) {
          if (endCell->end != End::None) numEndpoints--;
        }

        if (numEndpoints == 0) {
          TailRecurse(cell);
          return;
        }
      }
    }
  }

  // Recursion order (LRUD) is optimized for BL->TR and mid-start puzzles
  if (y%2 == 0) {
    path->UnsafePush(PATH_LEFT);
    SolveLoop(x - 1, y, solutionPaths, numEndpoints);
    path->Pop();

    path->UnsafePush(PATH_RIGHT);
    SolveLoop(x + 1, y, solutionPaths, numEndpoints);
    path->Pop();
  }

  if (x%2 == 0) {
    path->UnsafePush(PATH_TOP);
    SolveLoop(x, y - 1, solutionPaths, numEndpoints);
    path->Pop();

    path->UnsafePush(PATH_BOTTOM);
    SolveLoop(x, y + 1, solutionPaths, numEndpoints);
    path->Pop();
  }

  TailRecurse(cell);
}
