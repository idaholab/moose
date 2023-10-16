//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "EigenADReal.h"
#include "ADRankTwoTensorForward.h"
#include "RankTwoTensor.h"

struct ADR2DataMapper
{
  ADR2DataMapper(ADRankTwoTensor & res) : _res(res) {}
  ADReal & operator()(int i, int j) const { return _res(i, j); }
  ADRankTwoTensor & _res;
};

TEST(EigenADRealTest, gebp_kernel)
{
  ADRankTwoTensor res1;
  ADR2DataMapper dm(res1);

  Eigen::internal::gebp_kernel<ADReal, ADReal, int, ADR2DataMapper, 1, 1, false, false> kernel;

  ADRealVectorValue A(1, 2, 3);
  ADRealVectorValue B(5, 7, 11);

  // compute res1 using the kernel
  kernel(dm, &A(0), &B(0), 3, 1, 3, 0.1);

  // compute res2 using the rank two tensor operators
  const auto res2 = ADRankTwoTensor::outerProduct(A, B) * 0.1;

  EXPECT_NEAR(MetaPhysicL::raw_value((res1 - res2).L2norm()), 0.0, 1e-9);
}
