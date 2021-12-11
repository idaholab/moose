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
#include "SymmetricRankFourTensor.h"
#include "RankFourTensor.h"

#include "libmesh/vector_value.h"

class SymmetricRankFourTensorTest : public ::testing::Test
{
protected:
  void SetUp()
  {
    _i1.resize(9);
    for (auto i : index_range(_i1))
      _i1[i] = i + 1.1 * (i % 2 ? 1 : -1);
    _s1.fillFromInputVector(_i1, SymmetricRankFourTensor::symmetric9);

    _i2.resize(21);
    for (auto i : index_range(_i2))
      _i2[i] = i + 1;
    _s2.fillFromInputVector(_i2, SymmetricRankFourTensor::symmetric21);

    for (auto i : make_range(6))
      for (auto j : make_range(6))
        _s3(i, j) = (i + 1) * 10 + j + 1;

    _v = RealVectorValue(1, 2, 3);
  }

  RealVectorValue _v;
  std::vector<Real> _i1;
  std::vector<Real> _i2;
  SymmetricRankFourTensor _s1;
  SymmetricRankFourTensor _s2;
  SymmetricRankFourTensor _s3;
};
