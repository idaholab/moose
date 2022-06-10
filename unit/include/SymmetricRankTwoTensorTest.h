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
#include "SymmetricRankTwoTensor.h"
#include "libmesh/vector_value.h"

class SymmetricRankTwoTensorTest : public ::testing::Test
{
protected:
  void SetUp()
  {
    _m0 = SymmetricRankTwoTensor(0, 0, 0, 0, 0, 0);
    _m1 = SymmetricRankTwoTensor(1, 1, 1, 0, 0, 0);
    _m2 = SymmetricRankTwoTensor(1, 2, 3, 0, 0, 0);
    _m3 = SymmetricRankTwoTensor(1, -5, 9, -6, 3, 2);
    _m4 = SymmetricRankTwoTensor(6, 5, 4, 3, 2, 1);

    _v = RealVectorValue(1, 2, 3);
  }

  SymmetricRankTwoTensor _m0;
  SymmetricRankTwoTensor _m1;
  SymmetricRankTwoTensor _m2;
  SymmetricRankTwoTensor _m3;
  SymmetricRankTwoTensor _m4;
  RealVectorValue _v;
};
