#include "Solve.h"
#include "Puzzle.h"
#include "Utilities.h"
#include "Validate.h"

using namespace std;

// @Volatile -- must match order of MOVE_* in trace2
#define PATH_NONE   0
#define PATH_LEFT   1
#define PATH_RIGHT  2
#define PATH_TOP    3
#define PATH_BOTTOM 4

Solver::Solver(Puzzle& puzzle_) {
  puzzle = &puzzle_;
}

vector<Path> Solver::Solve(int MAX_SOLUTIONS) {
  path.clear(); // @Redundant
  doPruning = false;

  vector<Cell*> startPoints;
  numEndpoints = 0;

  puzzle->_hasNegations = false;
  puzzle->_hasPolyominos = false;
  for (int x=0; x<puzzle->_width; x++) {
    for (int y=0; y<puzzle->_height; y++) {
      Cell* cell = &puzzle->_grid[x][y];
      if (cell->type.empty()) continue;
      if (cell->start == true) {
        startPoints.push_back(cell);
      }
      if (!cell->end.empty()) numEndpoints++;
      if (cell->type == "nega") puzzle->_hasNegations = true;
      if (cell->type == "poly" || cell->type == "ylop") puzzle->_hasPolyominos = true;
    }
  }

  solutionPaths.clear();
  // Some reasonable default data, which will avoid crashes during the solveLoop.
  // var earlyExitData = [false, {"isEdge": false}, {"isEdge": false}]
  if (MAX_SOLUTIONS == 0) MAX_SOLUTIONS = 10000;

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
  // doPruning = (puzzle->_pillar == false)

  for (Cell* startPoint : startPoints) {
    // NOTE: This is subtly different from WitnessPuzzles, which starts the path with [[x, y]] instead of [x, y]!
    path = { startPoint->x, startPoint->y };
    puzzle->_startPoint = startPoint;
    SolveLoop(startPoint->x, startPoint->y);
  }

  return solutionPaths;
}

void Solver::TailRecurse(Cell* cell) {
  cell->line = LINE_NONE;
  /*
  if (puzzle->symmetry != null) {
    Cell* sym = puzzle->GetSymmetricalCell(cell->x, cell->y);
    sym->line = LINE_NONE;
  }
  */
}


void Solver::SolveLoop(int x, int y) {
  // Stop trying to solve once we reach our goal
  if (solutionPaths.size() >= MAX_SOLUTIONS) return;

  // Check for collisions (outside, gap, self, other)
  Cell* cell = puzzle->GetCell(x, y);
  if (cell == nullptr || cell->type.empty()) return;
  if (cell->gap > GAP_NONE) return;
  if (cell->line != LINE_NONE) return;

  // if (puzzle->symmetry == null) {
    cell->line = LINE_BLACK;
  /*} else {
    var sym = puzzle->getSymmetricalPos(x, y)
    if (puzzle->matchesSymmetricalPos(x, y, sym.x, sym.y)) return // Would collide with our reflection

    var symCell = puzzle->getCell(sym.x, sym.y)
    if (symCell.gap > window.GAP_NONE) return

    puzzle->updateCell2(x, y, 'line', window.LINE_BLUE)
    puzzle->updateCell2(sym.x, sym.y, 'line', window.LINE_YELLOW)
  }*/

  if (!cell->end.empty()) {
    path.push_back(PATH_NONE);
    puzzle->_endPoint = cell;
    Validator::Validate(*puzzle, true);
    if (puzzle->_valid) solutionPaths.push_back(path);
    path.pop_back();

    // If there are no further endpoints, tail recurse.
    // Otherwise, keep going -- we might be able to reach another endpoint.
    numEndpoints--;
    if (numEndpoints == 0) {
      TailRecurse(cell);
      return;
    }
  }

  /*
  if (doPruning) {
    var isEdge = x <= 0 || y <= 0 || x >= puzzle->width - 1 || y >= puzzle->height - 1
    var newEarlyExitData = [
      earlyExitData[0] || (!isEdge && earlyExitData[2].isEdge), // Have we ever left an edge?
      earlyExitData[2],                                         // The position before our current one
      {'x':x, 'y':y, 'isEdge':isEdge}                           // Our current position.
    ]
    if (earlyExitData[0] && !earlyExitData[1].isEdge && earlyExitData[2].isEdge && isEdge) {
      // See the above comment for an explanation of this math.
      var floodX = earlyExitData[2].x + (earlyExitData[1].x - x)
      var floodY = earlyExitData[2].y + (earlyExitData[1].y - y)
      var region = puzzle->getRegion(floodX, floodY)
      if (region != null) {
        var regionData = window.validateRegion(puzzle, region, true)
        if (!regionData.valid()) return tailRecurse(x, y)

        // Additionally, we might have left an endpoint in the enclosed region.
        // If so, we should decrement the number of remaining endpoints (and possibly tail recurse).
        for (var pos of region.cells) {
          var endCell = puzzle->getCell(pos.x, pos.y)
          if (endCell != null && endCell.end != null) numEndpoints--
        }

        if (numEndpoints === 0) return tailRecurse(x, y)
      }
    }
  } else {
    var newEarlyExitData = earlyExitData // Unused, just make a cheap copy.
  }
  */

  path.push_back(PATH_NONE);

  // Recursion order (LRUD) is optimized for BL->TR and mid-start puzzles
  if (y%2 == 0) {
    *path.end() = PATH_LEFT;
    SolveLoop(x - 1, y);

    *path.end() = PATH_RIGHT;
    SolveLoop(x + 1, y);
  }

  if (x%2 == 0) {
    *path.end() = PATH_TOP;
    SolveLoop(x, y - 1);

    *path.end() = PATH_BOTTOM;
    SolveLoop(x, y + 1);
  }

  path.pop_back();
  TailRecurse(cell);
}
