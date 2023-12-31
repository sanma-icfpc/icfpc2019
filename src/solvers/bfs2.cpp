#include <iostream>
#include <cctype>

#include "map_parse.h"
#include "solver_registry.h"

std::string bfs2Solver(SolverParam param, Game* game, SolverIterCallback iter_callback) {
  while (true) {
    Wrapper* w = game->wrappers[0].get();
    const std::vector<Trajectory> trajs = map_parse::findNearestUnwrapped(*game, w->pos, DISTANCE_INF);
    int count = game->countUnwrapped();
    if (trajs.size() == 0)
      break;
    for(auto t : trajs){
      const char c = Direction2Char(t.last_move);
      w->move(c);
      game->tick();
      displayAndWait(param, game);
      if (iter_callback && !iter_callback(game)) return game->getCommand();
      if (count != game->countUnwrapped()) {
        break;
      }
    }
  }
  return game->getCommand();
}

REGISTER_SOLVER("bfs2", bfs2Solver);
