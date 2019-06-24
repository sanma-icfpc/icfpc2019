#include "game.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <limits>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>
#include <experimental/filesystem>

#include "fill_polygon.h"
#include "manipulator_reach.h"

Buy::Buy() {
  for (int i = 0; i < BoosterType::N; ++i) {
    boosters[i] = 0;
  }
}
Buy::Buy(const std::string& buy_desc) : Buy() {
  for (char ch : buy_desc) {
    switch (ch) {
      case 'B': boosters[BoosterType::MANIPULATOR] += 1; break;
      case 'F': boosters[BoosterType::FAST_WHEEL] += 1; break;
      case 'L': boosters[BoosterType::DRILL] += 1; break;
      case 'R': boosters[BoosterType::TELEPORT] += 1; break;
      case 'C': boosters[BoosterType::CLONING] += 1; break;
      case '\r':
      case '\n':
      case ' ':
        break;
      default:
        assert (false);
    }
  }
}
Buy Buy::fromFile(const std::string& file_path) {
  assert (std::experimental::filesystem::is_regular_file(file_path));
  std::ifstream ifs(file_path);
  std::string str((std::istreambuf_iterator<char>(ifs)),
                  std::istreambuf_iterator<char>());
  return Buy(str);
}

Game::Game() {
  for (int i = 0; i < BoosterType::N; ++i) {
    num_boosters[i] = {};
  }
}

Game::Game(const std::string& task) : Game() {
  ParsedMap parsed = parseDescString(task);
  map2d = parsed.map2d;

  auto w = std::make_unique<Wrapper>(this, parsed.wrappy, 0);
  pick(*w, nullptr);
  paint(*w, nullptr);
  wrappers.push_back(std::move(w));
}

Game::Game(const std::vector<std::string>& mp) : Game() {
  ParsedMap parsed = parseMapString(mp);
  map2d = parsed.map2d;

  auto w = std::make_unique<Wrapper>(this, parsed.wrappy, 0);
  pick(*w, nullptr);
  paint(*w, nullptr);
  wrappers.push_back(std::move(w));
}

Game::Game(const Game& another) : Game() {
  operator=(another);
}

Game& Game::operator=(const Game& rhs) {
  time = rhs.time;
  map2d = rhs.map2d;
  num_boosters = rhs.num_boosters;
  debug_keyvalues = rhs.debug_keyvalues;
  for (auto& rhs : rhs.wrappers) {
    auto w = std::make_unique<Wrapper>(this, rhs->pos, rhs->index);
    *(w.get()) = *(rhs.get());
    w->game = this;
    wrappers.emplace_back(std::move(w));
  }
  for (auto& rhs : rhs.next_wrappers) {
    auto w = std::make_unique<Wrapper>(this, rhs->pos, rhs->index);
    *(w.get()) = *(rhs.get());
    w->game = this;
    next_wrappers.emplace_back(std::move(w));
  }
}

void Game::buyBoosters(const Buy& buy) {
  for (int i = 0; i < BoosterType::N; ++i) {
    num_boosters[i] += buy.boosters[i];
  }
}

bool Game::tick() {
  // make sure all wrappers has provided a command.
#ifndef NDEBUG
  for (auto& wrapper : wrappers) {
    assert (!wrapper->actions.empty());
    assert (wrapper->actions.back().timestamp == time + 1);
  }
#endif  // !defined(NDEBUG)
  ++time;
  // add new wrappers.
  for (auto&& w : next_wrappers) {
    wrappers.push_back(std::move(w));
  }
  next_wrappers.clear();
  return true;
}

void Game::pick(const Wrapper& w, Action* a_optional) {
  // automatically pick up boosters with no additional time cost.
  for (auto booster : boosters) {
    if (map2d(w.pos) & booster.map_bit) {
      if (a_optional) {
        assert (booster.booster_type < a_optional->pick_boosters.size());
        a_optional->pick_boosters[booster.booster_type].push_back(w.pos);
      }
      assert (booster.booster_type < num_boosters.size());
      ++num_boosters[booster.booster_type];
      map2d(w.pos) &= ~booster.map_bit;
    }
  }
}

void Game::paint(const Wrapper& w, Action* a_optional) {
  auto p = w.pos;
  assert (map2d.isInside(p));

  // paint
  if ((map2d(p) & CellType::kWrappedBit) == 0) {
    map2d(p) |= CellType::kWrappedBit;
    --map2d.num_unwrapped;
    if (a_optional) a_optional->absolute_new_wrapped_positions.push_back(p);
  }

  // paint manipulator
  for (auto manip : absolutePositionOfReachableManipulators(map2d, p, w.manipulators)) {
    if ((map2d(manip) & CellType::kWrappedBit) == 0) {
      if (a_optional) a_optional->absolute_new_wrapped_positions.push_back(manip);
      map2d(manip) |= CellType::kWrappedBit;
      --map2d.num_unwrapped;
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
  return countUnwrapped() == 0;
}

std::vector<Point> Game::getWrapperPositions() const {
  std::vector<Point> wrapper_positions;
  for (auto& w : wrappers) {
    wrapper_positions.push_back(w->pos);
  }
  return wrapper_positions;
}

void Game::addClonedWrapperForNextFrame(std::unique_ptr<Wrapper> wrapper) { 
  next_wrappers.push_back(std::move(wrapper));
}

bool checkCommandString(std::string cmd) {
  auto check_noexist = [&](std::string pattern) {
    if (cmd.find(pattern) != std::string::npos) {
      std::cerr << "Suspicious command: " << pattern << std::endl;
      return false;
    }
    return true;
  };

  bool okay = true;
  okay = check_noexist("WSWS") && okay;
  okay = check_noexist("ADAD") && okay;
  okay = check_noexist("EQEQ") && okay;
  return okay;
}

std::ostream& operator<<(std::ostream& os, const Game& game) {
  os << "Time: " << game.time << "\n";
  const int W = game.map2d.W, H = game.map2d.H;
  const int window_w = 100;
  const int window_h = 30;
  if (W < window_w && H < window_h) {
    // whole map
    for (auto& line : dumpMapString(game.map2d, game.getWrapperPositions())) {
      os << line << "\n";
    }
  } else {
    // in window
    const int fx = std::max(0, game.wrappers[0]->pos.x - window_w);
    const int tx = std::min(W, game.wrappers[0]->pos.x + window_w);
    const int fy = std::max(0, game.wrappers[0]->pos.y - window_h);
    const int ty = std::min(H, game.wrappers[0]->pos.y + window_h);
    std::vector<Point> wrappers_in_window;
    for (auto p : game.getWrapperPositions()) {
      wrappers_in_window.push_back({p.x - fx, p.y - fy});
    }
    for (auto& line : dumpMapString(game.map2d.slice(fx, tx, fy, ty), wrappers_in_window)) {
      os << line << "\n";
    }
  }
  os << "Unwrapped: " << game.map2d.num_unwrapped << "\n";

  os << "Boosters: B(" << game.num_boosters[BoosterType::MANIPULATOR] << ") "
     << "F(" << game.num_boosters[BoosterType::FAST_WHEEL] << ") "
     << "L(" << game.num_boosters[BoosterType::DRILL] << ") "
     << "C(" << game.num_boosters[BoosterType::CLONING] << ") "
     << "R(" << game.num_boosters[BoosterType::TELEPORT] << ")\n";
  os << "Wrappers: " << game.wrappers.size() << "\n";
  for (auto& w : game.wrappers) {
    os << w->index << " : ";
    os << "Dir[";
    switch (w->direction) {
      case Direction::W: os << "↑"; break;
      case Direction::A: os << "←"; break;
      case Direction::S: os << "↓"; break;
      case Direction::D: os << "→"; break;
    }
    os << "]";
    if (w->time_fast_wheels > 0) {
      os << " Speedup (" << w->time_fast_wheels << ")\n";
    }
    if (w->time_drill > 0) {
      os << " Drill (" << w->time_drill << ")\n";
    }
    os << "\n";
  }
  os << "Commad: " << game.getCommand().size() << "\n";
  //os << "Commad: " << game.getCommand() << "\n";
  for (auto kv : game.debug_keyvalues) {
    os << kv.first << "=" << kv.second << " ";
  }
  os << "\n";

  return os;
}
