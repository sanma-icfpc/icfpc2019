#include "game_map.h"

#include <iostream>
#include <ostream>
#include <algorithm>
#include <limits>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <sstream>

#include "fill_polygon.h"

namespace {
Point ParsePoint(char*& p) {
  assert (*p == '(');
  Point pos;
  int x = std::strtol(++p, &p, 10);
  assert (*p == ',');
  int y = std::strtol(++p, &p, 10);
  assert (*p == ')');
  ++p;

  return {x, y};
}

std::vector<Point> ParseMap(char*& p) {
  std::vector<Point> map_pos;

  assert (*p == '(');
  map_pos.emplace_back(ParsePoint(p));
  while (*p == ',') {
    map_pos.emplace_back(ParsePoint(++p));
  }

  return map_pos;
}

std::vector<std::vector<Point>> ParseObstacles(char*& p) {
  std::vector<std::vector<Point>> obstacles;
  assert (*p == '(');
  obstacles.emplace_back(ParseMap(p));
  while (*p == ';') {
    obstacles.emplace_back(ParseMap(++p));
  }
  return obstacles;
}

Booster ParseBooster(char*& p) {
  assert (std::strchr("BFLXR", *p) != nullptr);
  char code = *p;
  Point point { ParsePoint(++p) };
  return {code, point};
}

std::vector<Booster> ParseBoosters(char*& p) {
  assert (std::strchr("BFLXR", *p) != nullptr);
  std::vector<Booster> boosters;
  boosters.emplace_back(ParseBooster(p));
  while (*p == ';') {
    boosters.emplace_back(ParseBooster(++p));
  }
  return boosters;
}

}  // namespace


ParsedMap parseDescString(std::string desc_string) {
  ParsedMap map;
  char* p = const_cast<char*>(desc_string.data());

  std::vector<Point> map_pos { ParseMap(p) };
  assert (*p == '#');
  map.wrappy = ParsePoint(++p);
  assert (*p == '#');
  std::vector<std::vector<Point>> obstacles { ParseObstacles(++p) };
  assert (*p == '#');
  std::vector<Booster> boosters { ParseBoosters(++p) };

  BoundingBox map_bbox = calcBoundingBox(map_pos);
  assert (map_bbox.lower.x >= 0);
  assert (map_bbox.lower.y >= 0);
  assert (map_bbox.isValid());
  map.map2d = Map2D(map_bbox.upper.x, map_bbox.upper.y, CellType::kObstacleBit);
  FillPolygon(map.map2d, map_pos, CellType::kEmpty);
  for (const auto& obstacle : obstacles) {
    FillPolygon(map.map2d, obstacle, CellType::kObstacleBit);
  }

  for (auto booster : boosters) {
    switch (booster.first) {
      case BOOSTER_MANIPULATOR:
        map.map2d(booster.second) |= CellType::kBoosterManipulatorBit;
        break;
      case BOOSTER_FAST_WHEEL:
        map.map2d(booster.second) |= CellType::kBoosterFastWheelBit;
        break;
      case BOOSTER_DRILL:
        map.map2d(booster.second) |= CellType::kBoosterDrillBit;
        break;
      case UNKNOWN:
        map.map2d(booster.second) |= CellType::kBoosterUnknownXBit;
        break;
    }
  }

  return map;
}

ParsedMap parseMapString(std::vector<std::string> map_strings_top_to_bottom) {
  ParsedMap map;
  std::vector<std::string> maplines = map_strings_top_to_bottom;
  std::reverse(maplines.begin(), maplines.end()); // now maplines[y] corresponds to y-line.

  int H = maplines.size();
  int W = maplines[0].size();
  map.map2d = Map2D(W, H, CellType::kEmpty);
  for (int y = 0; y < H; ++y) {
    for (int x = 0; x < W; ++x) {
      switch (maplines[y][x]) {
        case NON_WRAPPED:
          map.map2d(x, y) = CellType::kEmpty;
          break;
        case WRAPPED:
          map.map2d(x, y) = CellType::kWrappedBit;
          break;
        case WRAPPY:
          map.map2d(x, y) = CellType::kEmpty;
          map.wrappy = {x, y};
          break;
        case BOOSTER_MANIPULATOR:
          map.map2d(x, y) = CellType::kBoosterManipulatorBit;
          break;
        case BOOSTER_FAST_WHEEL:
          map.map2d(x, y) = CellType::kBoosterFastWheelBit;
          break;
        case BOOSTER_DRILL:
          map.map2d(x, y) = CellType::kBoosterDrillBit;
          break;
        case WALL:
          map.map2d(x, y) = CellType::kObstacleBit;
          break;
        case UNKNOWN:
          map.map2d(x, y) = CellType::kBoosterUnknownXBit;
          break;
        default:
          assert(false);
          break;
      }
    }
  }

  return map;
}

std::vector<std::string> dumpMapString(const Map2D& map2d, Point wrappy) {
  std::vector<std::vector<char>> charmap;

  for (int y = 0; y < map2d.H; ++y) {
    std::vector<char> line(map2d.W, WALL);
    for (int x = 0; x < map2d.W; ++x) {
      char c = NON_WRAPPED;
      if (map2d(x, y) & CellType::kWrappedBit) { c = WRAPPED; }
      if (map2d(x, y) & CellType::kBoosterManipulatorBit) { c = BOOSTER_MANIPULATOR; }
      if (map2d(x, y) & CellType::kBoosterFastWheelBit) { c = BOOSTER_FAST_WHEEL; }
      if (map2d(x, y) & CellType::kBoosterDrillBit) { c = BOOSTER_DRILL; }
      if (map2d(x, y) & CellType::kBoosterUnknownXBit) { c = UNKNOWN; }
      if (map2d(x, y) & CellType::kBoosterTeleportBit) { c = BOOSTER_TELEPORT; }
      if (map2d(x, y) & CellType::kObstacleBit) { c = WALL; } // highest priority
      line[x] = c;
    }
    charmap.push_back(line);
  }

  charmap[wrappy.y][wrappy.x] = WRAPPY;

  std::vector<std::string> result;
  for (auto& line : charmap) {
    line.push_back('\0');
    result.push_back(std::string(line.begin(), line.end()));
  }
  return result;
}

std::ostream& operator<<(std::ostream& os, const Map2D& map) {
    for (int y = 0; y < map.H; ++y) {
        for (int x = 0; x < map.W; ++x) {
            std::cout << map(x, y);
        }
        std::cout << std::endl;
    }
    return os;
}