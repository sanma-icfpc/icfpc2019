LIB_PATH=../libs
BUILD_PATH=build
GTEST_DIR=$(LIB_PATH)/googletest/googletest

CXXFLAGS+=-O2 -Wall -Wno-sign-compare -std=c++14 -march=native
CXXFLAGS+=-I. -I$(GTEST_DIR) -I$(GTEST_DIR)/include -I$(LIB_PATH)/CLI11/include
#CXXFLAGS+=-g
#CXXFLAGS+=-DNDEBUG

LDFLAGS=-lstdc++fs

SRCS=base.cpp getch.cpp map2d.cpp booster.cpp wrapper.cpp game.cpp action.cpp solver_registry.cpp solver_helper.cpp solver_utils.cpp bits.cpp
SRCS+=puzzle.cpp
SRCS+=manipulator_reach.cpp fill_polygon.cpp
SRCS+=map_parse.cpp trajectory.cpp
OBJS=$(SRCS:%.cpp=$(BUILD_PATH)/%.o)

SOLVER_SRCS=$(wildcard solvers/*.cpp)
SOLVER_OBJS=$(SOLVER_SRCS:%.cpp=$(BUILD_PATH)/%.o)
SOLVER_BUILD_PATH=build/solvers

PUZZLE_SOLVER_SRCS=$(wildcard puzzle_solvers/*.cpp)
PUZZLE_SOLVER_OBJS=$(PUZZLE_SOLVER_SRCS:%.cpp=$(BUILD_PATH)/%.o)
PUZZLE_SOLVER_BUILD_PATH=build/puzzle_solvers

GTEST_SRCS=$(GTEST_DIR)/src/gtest-all.cc $(GTEST_DIR)/src/gtest_main.cc
TEST_BUILD_PATH=build/tests
TEST_LDFLAGS=$(LDFLAGS) -lpthread
TEST_SRCS=$(wildcard tests/*.cpp)
TEST_OBJS=$(TEST_SRCS:%.cpp=$(BUILD_PATH)/%.o)

TARGETS=solver test

.PHONY: all
all: dirs $(TARGETS)

solver: main.cpp $(OBJS) $(SOLVER_OBJS) $(PUZZLE_SOLVER_OBJS)
	$(CXX) $(CXXFLAGS) -Wall $^ -o $@ $(LDFLAGS)

.PHONY: dirs
dirs:
	@mkdir -p $(BUILD_PATH) $(SOLVER_BUILD_PATH) $(PUZZLE_SOLVER_BUILD_PATH) $(TEST_BUILD_PATH)

$(BUILD_PATH)/%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -fr $(BUILD_PATH) $(TARGETS)

test: $(GTEST_SRCS) $(TEST_OBJS) $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(TEST_LDFLAGS)

$(BUILD_PATH)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
