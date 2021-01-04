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
  }

  RankTwoTensor _m0;
  RankTwoTensor _m1;
  RankTwoTensor _m2;
  RankTwoTensor _m3;
  RankTwoTensor _unsymmetric0;
  RankTwoTensor _unsymmetric1;
};
