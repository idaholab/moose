//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "gtest_include.h"

// Moose includes
#include "RankTwoTensor.h"
#include "RankThreeTensor.h"
#include "RankFourTensor.h"
#include "libmesh/vector_value.h"

class RankTwoTensorTest : public ::testing::Test
{
protected:
  void SetUp()
  {
    _m0 = RankTwoTensor(0, 0, 0, 0, 0, 0, 0, 0, 0);
    _m1 = RankTwoTensor(1, 0, 0, 0, 1, 0, 0, 0, 1);
    _m2 = RankTwoTensor(1, 0, 0, 0, 2, 0, 0, 0, 3);
    _m3 = RankTwoTensor(1, 2, 3, 2, -5, -6, 3, -6, 9);
    _unsymmetric0 = RankTwoTensor(1, 2, 3, -4, -5, -6, 7, 8, 9);
    _unsymmetric1 = RankTwoTensor(1, 2, 3, -4, -5, -6, 7, 8, 10);

    _v = RealVectorValue(1, 2, 3);
    _r3.fillFromInputVector({1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
                             15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27},
                            RankThreeTensor::general);

    _r4.fillFromInputVector({1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17,
                             18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34,
                             35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
                             52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68,
                             69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81},
                            RankFourTensor::general);
  }

  RankTwoTensor _m0;
  RankTwoTensor _m1;
  RankTwoTensor _m2;
  RankTwoTensor _m3;
  RankTwoTensor _unsymmetric0;
  RankTwoTensor _unsymmetric1;
  RealVectorValue _v;
  RankThreeTensor _r3;
  RankFourTensor _r4;

protected:
  static constexpr auto N = RankTwoTensor::N;
};
