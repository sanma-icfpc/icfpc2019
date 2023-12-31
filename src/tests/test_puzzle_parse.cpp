#include "../puzzle.h"

#include <ostream>
#include <iostream>
#include <gtest/gtest.h>

#include "../base.h"

TEST(PuzzleTest, ParseDesc) {
  // puzzle.cond
  Puzzle parsed = parsePuzzleCondString(
    "1,1,150,400,1200,6,10,5,1,3,4#"
    "(73,61),(49,125),(73,110),(98,49),(126,89),(68,102),(51,132),(101,123),(22,132),(71,120),(97,129),(118,76),(85,100),(88,22),(84,144),(93,110),(96,93),(113,138),(91,52),(27,128),(84,140),(93,143),(83,17),(123,85),(50,74),(139,97),(101,110),(77,56),(86,23),(117,59),(133,126),(83,135),(76,90),(70,12),(12,141),(116,87),(102,76),(19,138),(86,129),(86,128),(83,60),(100,98),(60,105),(61,103),(94,99),(130,124),(141,132),(68,84),(86,143),(72,119)#"
    "(145,82),(20,65),(138,99),(38,137),(85,8),(125,104),(117,48),(57,48),(64,119),(3,25),(40,22),(82,54),(121,119),(1,34),(43,98),(97,120),(10,90),(15,32),(41,13),(86,40),(3,83),(2,127),(4,40),(139,18),(96,49),(53,22),(5,103),(112,33),(38,47),(16,121),(133,99),(113,45),(50,5),(94,144),(16,0),(93,113),(18,141),(36,25),(56,120),(3,126),(143,144),(99,62),(144,117),(48,97),(69,9),(0,9),(141,16),(55,68),(81,3),(47,53)");
  
  Puzzle ground_truth;
  ground_truth.bNum = 1;
  ground_truth.eNum = 1;
  ground_truth.tSize = 150;
  ground_truth.vMin = 400;
  ground_truth.vMax = 1200;
  ground_truth.mNum = 6;
  ground_truth.fNum = 10;
  ground_truth.dNum = 5;
  ground_truth.rNum = 1;
  ground_truth.cNum = 3;
  ground_truth.xNum = 4;
  ground_truth.iSqs = {{73,61},{49,125},{73,110},{98,49},{126,89},{68,102},{51,132},{101,123},{22,132},{71,120},{97,129},{118,76},{85,100},{88,22},{84,144},{93,110},{96,93},{113,138},{91,52},{27,128},{84,140},{93,143},{83,17},{123,85},{50,74},{139,97},{101,110},{77,56},{86,23},{117,59},{133,126},{83,135},{76,90},{70,12},{12,141},{116,87},{102,76},{19,138},{86,129},{86,128},{83,60},{100,98},{60,105},{61,103},{94,99},{130,124},{141,132},{68,84},{86,143},{72,119}};
  ground_truth.oSqs = {{145,82},{20,65},{138,99},{38,137},{85,8},{125,104},{117,48},{57,48},{64,119},{3,25},{40,22},{82,54},{121,119},{1,34},{43,98},{97,120},{10,90},{15,32},{41,13},{86,40},{3,83},{2,127},{4,40},{139,18},{96,49},{53,22},{5,103},{112,33},{38,47},{16,121},{133,99},{113,45},{50,5},{94,144},{16,0},{93,113},{18,141},{36,25},{56,120},{3,126},{143,144},{99,62},{144,117},{48,97},{69,9},{0,9},{141,16},{55,68},{81,3},{47,53}};

  std::cout << parsed << std::endl;
  EXPECT_EQ(parsed, ground_truth);
}

TEST(PuzzleTest, constraintToMap) {
  Puzzle parsed = parsePuzzleCondString(
    "1,1,10,400,1200,6,10,5,1,3,4#"
    "(0,2),(8,3)#"
    "(0,3),(9,4)");
  
  auto constraint_map = parsed.constraintsToMap();
  for (auto line : dumpPuzzleConstraintMapString(constraint_map)) {
    std::cout << line << std::endl;
  }
  Map2D ground_truth(10, 10, {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    2, 0, 0, 0, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  });
  EXPECT_EQ(constraint_map, ground_truth);
}

TEST(PuzzleTest, puzzleSolutionToString) {
  PuzzleSolution sol;
  sol.wall = {{0, 0}, {4, 0}, {4, 4}, {0, 4}};
  sol.wrapper = {1, 1};
  sol.Bs = {{1, 2}};
  sol.Fs = {{1, 3}};
  sol.Ls = {{2, 0}};
  sol.Rs = {{2, 1}};
  sol.Cs = {{2, 2}};
  sol.Xs = {{2, 3}};
  //std::cout << sol.toString() << std::endl;
  EXPECT_EQ(
    "(0,0),(4,0),(4,4),(0,4)#(1,1)##B(1,2);F(1,3);L(2,0);R(2,1);C(2,2);X(2,3)",
    sol.toString());
}