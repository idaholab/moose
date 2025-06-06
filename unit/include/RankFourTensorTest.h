//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "gtest_include.h"

// Moose includes
#include "RankFourTensor.h"
#include "libmesh/vector_value.h"

class RankFourTensorTest : public ::testing::Test
{
protected:
  void SetUp()
  {
    _v = RealVectorValue(1, 2, 3);
    _r4.fillFromInputVector({1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17,
                             18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34,
                             35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
                             52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68,
                             69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81},
                            RankFourTensor::general);
  }

  RealVectorValue _v;
  RankFourTensor _r4;
};
