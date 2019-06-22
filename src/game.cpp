#include "game.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "fill_polygon.h"
#include "manipulator_reach.h"

Game::Game() {
  for (int i = 0; i < BoosterType::N; ++i) {
    num_boosters[i] = {};
  }
}

Game::Game(const std::string& task) : Game() {
  ParsedMap parsed = parseDescString(task);
  map2d = parsed.map2d;

  auto w = std::make_shared<Wrapper>(this, parsed.wrappy, 0);
  paintAndPick(*w, nullptr);
  wrappers.push_back(w);
}

Game::Game(const std::vector<std::string>& mp) : Game() {
  ParsedMap parsed = parseMapString(mp);
  map2d = parsed.map2d;

  auto w = std::make_shared<Wrapper>(this, parsed.wrappy, 0);
  paintAndPick(*w, nullptr);
  wrappers.push_back(w);
}

bool Game::tick() {
  // make sure all wrappers has provided a command.
  for (auto& wrapper : wrappers) {
    assert (!wrapper->actions.empty());
    assert (wrapper->actions.back().timestamp == time + 1);
  }
  ++time;
  return true;
}

void Game::paintAndPick(const Wrapper& w, Action* a_optional) {
  auto p = w.pos;
  assert (map2d.isInside(p));

  // paint
  if ((map2d(p) & CellType::kWrappedBit) == 0) {
    map2d(p) |= CellType::kWrappedBit;
    if (a_optional) a_optional->absolute_new_wrapped_positions.push_back(p);
  }

  // paint manipulator
  for (auto manip : absolutePositionOfReachableManipulators(map2d, p, w.manipulators)) {
    if ((map2d(manip) & CellType::kWrappedBit) == 0) {
      if (a_optional) a_optional->absolute_new_wrapped_positions.push_back(manip);
      map2d(manip) |= CellType::kWrappedBit;
    }
  }

  // automatically pick up boosters with no additional time cost.
  for (auto booster : boosters) {
    if (map2d(p) & booster.map_bit) {
      if (a_optional) {
        assert (booster.booster_type < a_optional->pick_boosters.size());
        a_optional->pick_boosters[booster.booster_type].push_back(p);
      }
      assert (booster.booster_type < num_boosters.size());
      ++num_boosters[booster.booster_type];
      map2d(p) &= ~booster.map_bit;
    }
  }
}

bool Game::undo() {
  if (time <= 0) return false;
  if (wrappers.empty()) return false;

  for (auto it = wrappers.begin(); it != wrappers.end();) {
    (*it)->undoAction();
    // unspawn.
    if ((*it)->actions.empty()) {
      it = wrappers.erase(it);
    } else {
      ++it;
    }
  }

  // undo time
  time -= 1;
  return true;
}

std::string Game::getCommand() const {
  std::ostringstream oss;
  for (int i = 0; i < wrappers.size(); ++i) {
    oss << wrappers[i]->getCommand();
    if (i + 1 < wrappers.size()) {
      oss << '#';
    }
  }
  return oss.str();
}

bool Game::isEnd() const {
  return countUnWrapped() == 0;
}

int Game::countUnWrapped() const {
  static const int kMask = CellType::kObstacleBit | CellType::kWrappedBit;
  const int H = map2d.H;
  const int W = map2d.W;
  int count = 0;
  for (int x = 0; x < W; ++x) {
    for (int y = 0; y < H; ++y) {
      if ((map2d(x, y) & kMask) == 0)
        ++count;
    }
  }
  return count;
}

std::vector<Point> Game::getWrapperPositions() const {
  std::vector<Point> wrapper_positions;
  for (auto& w : wrappers) {
    wrapper_positions.push_back(w->pos);
  }
  return wrapper_positions;
}

std::ostream& operator<<(std::ostream& os, const Game& game) {
  os << "Time: " << game.time << "\n";
  for (auto& line : dumpMapString(game.map2d, game.getWrapperPositions())) {
    os << line << "\n";
  }

  os << "Boosters: B(" << game.num_boosters[BoosterType::MANIPULATOR] << ") "
     << "F(" << game.num_boosters[BoosterType::FAST_WHEEL] << ") "
     << "L(" << game.num_boosters[BoosterType::DRILL] << ") "
     << "C(" << game.num_boosters[BoosterType::CLONING] << ") "
     << "R(" << game.num_boosters[BoosterType::TELEPORT] << ")\n";
  os << "Wrappers: " << game.wrappers.size() << "\n";
  for (auto& w : game.wrappers) {
    os << w->index << " : ";
    if (w->time_fast_wheels > 0) {
      os << " Speedup (" << w->time_fast_wheels << ")\n";
    }
    if (w->time_drill > 0) {
      os << " Drill (" << w->time_drill << ")\n";
    }
    os << "\n";
  }
  os << "Commad: " << game.getCommand() << "\n";

  return os;
}
