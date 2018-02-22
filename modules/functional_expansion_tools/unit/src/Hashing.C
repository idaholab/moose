//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "Hashing.h"

TEST(FunctionalExpansionsTest, hashPoint)
{
  const Point location(0.9780619323, 0.2136556650, 0.3509044429);
  const Point location_mirror(location);
  Point location_twiddle_limit(location);
  Point location_twiddle_nochange(location);

  location_twiddle_limit(0) += 6e-17;
  location_twiddle_nochange(0) += 5e-17;

  const hashing::HashValue result = hashing::hashCombine(location);
  const hashing::HashValue result_mirror = hashing::hashCombine(location_mirror);
  const hashing::HashValue result_twiddle_limit = hashing::hashCombine(location_twiddle_limit);
  const hashing::HashValue result_twiddle_nochange =
      hashing::hashCombine(location_twiddle_nochange);

  EXPECT_EQ(result, result_mirror);
  EXPECT_NE(result, result_twiddle_limit);
  EXPECT_EQ(result, result_twiddle_nochange);
}

TEST(FunctionalExpansionsTest, hashPointAndTime)
{
  const Real time = 0.5049258208;
  const Point location(0.8392988091, 0.8482835642, 0.2509438471);
  const Point location_mirror(location);
  Point location_twiddle_limit(location);
  Point location_twiddle_nochange(location);

  location_twiddle_limit(0) += 6e-17;
  location_twiddle_nochange(0) += 5e-17;

  const hashing::HashValue result = hashing::hashCombine(time, location);
  const hashing::HashValue result_mirror = hashing::hashCombine(time, location_mirror);
  const hashing::HashValue result_twiddle_limit =
      hashing::hashCombine(time, location_twiddle_limit);
  const hashing::HashValue result_twiddle_nochange =
      hashing::hashCombine(time, location_twiddle_nochange);

  EXPECT_EQ(result, result_mirror);
  EXPECT_NE(result, result_twiddle_limit);
  EXPECT_EQ(result, result_twiddle_nochange);
}

TEST(FunctionalExpansionsTest, hashIntVector)
{
  const std::vector<int> vector = {{351,  456, 49,   140,  491,  -482, 86,   314,  -442, 154,
                                    430,  231, 47,   -261, -64,  -126, -187, 12,   61,   -171,
                                    -145, 76,  -470, -100, 367,  1,    485,  458,  88,   112,
                                    -212, 357, -403, 467,  127,  138,  388,  -244, -479, -239,
                                    -354, 475, -453, -36,  -365, -248, -95,  93,   -286, 436}};
  const std::vector<int> vector_mirror(vector.begin(), vector.end());
  const std::vector<int> vector_chopped(vector.begin(), --vector.end());
  std::vector<int> vector_twiddle(vector.begin(), vector.end());
  ++vector_twiddle[0];

  const hashing::HashValue result = hashing::hashLargeContainer(vector);
  const hashing::HashValue result_mirror = hashing::hashLargeContainer(vector_mirror);
  const hashing::HashValue result_chopped = hashing::hashLargeContainer(vector_chopped);
  const hashing::HashValue result_twiddle = hashing::hashLargeContainer(vector_twiddle);

  EXPECT_EQ(result, result_mirror);
  EXPECT_NE(result, result_chopped);
  EXPECT_NE(result, result_twiddle);
}

TEST(FunctionalExpansionsTest, hashRealVector)
{
  const std::vector<Real> vector = {
      {0.7436426667, 0.6962231856, 0.7257906803, 0.8084661009, 0.2137680124, 0.1470705959,
       0.3657163957, 0.8818058481, 0.5513600342, 0.1984376524, 0.4276321100, 0.4177158632,
       0.5599816732, 0.5976773302, 0.0534853375, 0.7162077056, 0.9344288478, 0.4947189992,
       0.6393245755, 0.6877651005, 0.9567775877, 0.8757637166, 0.3850183877, 0.6800440879,
       0.5090953855, 0.7779340857, 0.2310272569, 0.7807447395, 0.3012011716, 0.2879436719,
       0.9785884888, 0.1201041026, 0.6422951422, 0.8657404100, 0.2686119524, 0.4199450789,
       0.2437974613, 0.3544349330, 0.5725840559, 0.2903856081, 0.0055479019, 0.6819050123,
       0.5512080507, 0.7301519914, 0.0077125671, 0.5284511770, 0.6894292950, 0.5014027958,
       0.9773264137, 0.8477810277}};
  const std::vector<Real> vector_mirror(vector.begin(), vector.end());
  const std::vector<Real> vector_chopped(vector.begin(), --vector.end());
  std::vector<Real> vector_twiddle_limit(vector.begin(), vector.end());
  std::vector<Real> vector_twiddle_nochange(vector.begin(), vector.end());

  vector_twiddle_limit[0] += 6e-17;
  vector_twiddle_nochange[0] += 5e-17;

  const hashing::HashValue result = hashing::hashLargeContainer(vector);
  const hashing::HashValue result_mirror = hashing::hashLargeContainer(vector_mirror);
  const hashing::HashValue result_chopped = hashing::hashLargeContainer(vector_chopped);
  const hashing::HashValue result_twiddle_limit = hashing::hashLargeContainer(vector_twiddle_limit);
  const hashing::HashValue result_twiddle_nochange =
      hashing::hashLargeContainer(vector_twiddle_nochange);

  EXPECT_EQ(result, result_mirror);
  EXPECT_NE(result, result_chopped);
  EXPECT_NE(result, result_twiddle_limit);
  EXPECT_EQ(result, result_twiddle_nochange);
}

TEST(FunctionalExpansionsTest, hashVararg)
{
  hashing::HashValue original = 42;
  hashing::HashValue additional_small, additional_regular, additional_large, original_mirror;

  hashing::hashCombine(original,
                       0.9873791320,
                       0.1953364838,
                       0.8116485930,
                       0.1863965161,
                       0.5928596550,
                       0.2295234343,
                       0.6904344651,
                       0.0045536257,
                       0.1940171658,
                       0.4950894997,
                       0.8079496584,
                       0.8060619760,
                       0.7486861178,
                       0.5493002792,
                       0.1596405782,
                       0.4023849890,
                       0.2782852666,
                       0.6461232825,
                       0.1064983494,
                       0.8130189389,
                       0.5726072736,
                       0.3327263263,
                       0.1472734104,
                       0.0234982033,
                       0.6812964288,
                       0.3276164827,
                       0.6911670346,
                       0.8299179444,
                       0.6484517577,
                       0.7986116002,
                       0.8813936466,
                       0.0049727250,
                       0.2010708901,
                       0.4933756641,
                       0.8354504016,
                       0.9452099799,
                       0.4643204087,
                       0.7382737011,
                       0.8045729776,
                       0.7870302766,
                       0.3384656050,
                       0.3401508297,
                       0.1595941894,
                       0.0673033342,
                       0.7483309385,
                       0.8940644866,
                       0.7948297883,
                       0.1890952442,
                       0.6151646001,
                       0.1672976625);

  additional_small = additional_regular = additional_large = original_mirror = original;
  hashing::hashCombine(additional_small, 3.9e-120);
  hashing::hashCombine(additional_regular, 0.3971072909);
  hashing::hashCombine(additional_large, 3.9e120);

  EXPECT_EQ(original, original_mirror);
  EXPECT_NE(original, additional_small);
  EXPECT_NE(original, additional_regular);
  EXPECT_NE(original, additional_large);
}
