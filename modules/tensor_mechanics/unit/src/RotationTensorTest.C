//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "RankFourTensor.h"
#include "RotationTensor.h"

TEST(RotationTensorTest, rotation)
{
  std::vector<Real> input(81);
  for (size_t i = 0; i < input.size(); ++i)
    input[i] = i + 1.1;

  RankFourTensor a(input, RankFourTensor::general);

  RotationTensor xrot(RotationTensor::XAXIS, 90);
  RotationTensor yrot(RotationTensor::YAXIS, 90);

  auto xyrot = xrot * yrot;
  auto yxrot = yrot * xrot;

  auto axy1 = a;
  axy1.rotate(xrot);
  axy1.rotate(yrot);

  auto axy2 = a;
  axy2.rotate(yxrot);

  EXPECT_NEAR(0, (axy1 - axy2).L2norm(), 1E-8);
}
