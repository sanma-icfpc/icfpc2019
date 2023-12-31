
#pragma once

#include <algorithm>
#include <ostream>
#include <string>
#include <vector>
#include <cassert>

#include "base.h"
#include "booster.h"

#define MAP2D_CHECK_INSIDE
#ifdef MAP2D_CHECK_INSIDE
# define MAP2D_ASSERT(eq) assert(eq)
#else
# define MAP2D_ASSERT(eq) 
#endif

// character representation of map ======================================
static const char NON_WRAPPED = '.';
static const char WRAPPED = ' ';
static const char WRAPPY = '@';
static const char BOOSTER_MANIPULATOR = 'B';
static const char BOOSTER_FAST_WHEEL = 'F';
static const char BOOSTER_DRILL = 'L';
static const char BOOSTER_TELEPORT = 'R';
static const char BOOSTER_CLONING = 'C';
static const char WALL = '#';
static const char SPAWN_POINT = 'X';
static const char TELEPORT_TARGET = 'T';

struct Map2D {
    using T = int;
    int W = 0;
    int H = 0;
    int num_unwrapped = 0;
    std::vector<T> data;

    Map2D() : Map2D(0, 0) {}
    Map2D(int W_, int H_, int value = 0)
      : W(std::max(W_, 0)),
        H(std::max(H_, 0)) {
        data.assign(W * H, value);
        if ((value & kUnwrappedMask) == 0) {
          num_unwrapped = W * H;
        }
    }
    Map2D(int W_, int H_, std::initializer_list<int> vals) : Map2D(W_, H_) {
        assert (vals.size() == W * H);
        auto it = vals.begin();
        for (int y = 0; y < H; ++y) {
            for (int x = 0; x < W; ++x, ++it) {
                (*this)(x, y) = *it;
                if ((*it & kUnwrappedMask) == 0) {
                    ++num_unwrapped;
                }
            }
        }
    }

    T& operator()(int x, int y) { MAP2D_ASSERT(isInside(x, y)); return data[y * W + x]; }
    T operator()(int x, int y) const { MAP2D_ASSERT(isInside(x, y)); return data[y * W + x]; }
    T& operator()(Point p) { MAP2D_ASSERT(isInside(p)); return data[p.y * W + p.x]; }
    T operator()(Point p) const { MAP2D_ASSERT(isInside(p)); return data[p.y * W + p.x]; }
    bool operator!=(const Map2D& rhs) const {
        return !operator==(rhs);
    }
    bool operator==(const Map2D& rhs) const {
        return W == rhs.W && H == rhs.H && data == rhs.data;
    }
    bool isInside(int x, int y) const {
        return 0 <= x && x < W && 0 <= y && y < H;
    }
    bool isInside(Point p) const {
        return isInside(p.x, p.y);
    }
    Map2D slice(int fx, int tx, int fy, int ty) const {
        if (fx < 0 || tx <= fx || W < tx) return {};
        if (fy < 0 || ty <= fy || H < ty) return {};
        Map2D sliced(tx - fx, ty - fy);
        for (int y = 0; y < sliced.H; ++y) {
            for (int x = 0; x < sliced.W; ++x) {
                sliced(x, y) = operator()(fx + x, fy + y);
            }
        }
        return sliced;
    }

    std::string toString(bool lower_origin, bool frame, int digits) const;

  private:
    static constexpr int kUnwrappedMask = CellType::kObstacleBit | CellType::kWrappedBit;
};

using Booster = std::pair<char, Point>;

// return all points (x, y) with (map(x, y) & mask) == bits
std::vector<Point> enumerateCellsByMask(const Map2D& map, int mask, int bits);
inline int countCellsByMask(const Map2D& map, int mask, int bits) { return enumerateCellsByMask(map, mask, bits).size(); }
std::vector<Point> findNearestPoints(const std::vector<Point>& haystack, Point needle);

// return true if there are no 8-connected pixel pairs which are not 4-connected.
// e.g.)
//  2 3 3
//  3 1 1 : false.
// e.g.)
//  2 2 3
//  3 1 1 : true.
bool isConnected4(const Map2D& map);

// return [start, ..., stop]
// if max_distance < 0, do not stop by distance.
std::vector<Point> shortestPathByMaskBFS(const Map2D& map,
  int free_mask, int free_bits,
  Point start,
  int target_mask, int target_bits,
  int max_distance = -1);

// return [start, ..., stop]
// if max_distance < 0, do not stop by distance.
std::vector<Point> shortestPathByMaskBFS(const Map2D& map,
  int free_mask, int free_bits,
  Point start, const std::vector<Point>& targets,
  int max_distance = -1);

struct ParsedMap {
    Map2D map2d;
    Point wrappy;
};
// parse *.desc string to construct Map2D and obtain other info.
ParsedMap parseDescString(std::string desc_string);

// parse *.map string to construct Map2D and obtain other info.
// map_strings_top_to_bottom[H - 1 - y] corresponds to the y-line.
ParsedMap parseMapString(std::vector<std::string> map_strings_top_to_bottom);

namespace detail {
char getMapChar(int map_bits);
}

// dump map string for display. the first row corresponds to the highest y.
// do not contain line terminator at the end of lines.
std::vector<std::string> dumpMapString(const Map2D& map2d, std::vector<Point> wrappy_list);

std::ostream& operator<<(std::ostream&, const Map2D&);