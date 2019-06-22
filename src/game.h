#pragma once

#include <ostream>
#include <string>
#include <vector>
#include <memory>

#include "base.h"
#include "map2d.h"
#include "wrapper.h"
#include "booster.h"

struct Game {
  Game(const std::string& desc); // initialize using a task description string from *.desc file.
  Game(const std::vector<std::string>& map); // initialize by a raster *.map file.

  // move time frame. make sure you provided commands for each wrapper.
  bool tick();

  // undo previous frame. if no actions are stacked, fail and return false.
  bool undo();

  // Returns True if no unwrapped cells are remained.
  bool isEnd() const;

  int countUnWrapped() const;

  std::string getCommand() const; // extended solution command.

  int nextWrapperIndex() const { return wrappers.size() + next_wrappers.size(); }
  void addClonedWrapperForNextFrame(std::unique_ptr<Wrapper> wrapper); // this wrapper will be available after tick()
  std::vector<Point> getWrapperPositions() const;

  // according to the rules, a tick consists of:
  // for i in [0..N]
  //   1. wrapper[i] picks anything in the cell
  //   2. wrapper[i] moves (e.g. use any boosters collected)
  void pick(const Wrapper& w, Action* a_optional); // helper func used by Wrapper
  void paint(const Wrapper& w, Action* a_optional); // helper func used by Wrapper

  // State of Game
  int time = 0;

  // shared & updated map
  Map2D map2d;

  // State of Wrappy ===================================
  std::vector<std::unique_ptr<Wrapper>> wrappers;

  // Unused boosters (shared among wrappers)
  std::array<int, BoosterType::N> num_boosters;

private:
  Game();
  std::vector<std::unique_ptr<Wrapper>> next_wrappers;
};

// Outputs game status
std::ostream& operator<<(std::ostream&, const Game&);
