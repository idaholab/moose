//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "FixedPoint.h"

TEST(FixedPoint, test)
{
  auto compute = [&](const Moose::FixedPoint::Value<2> & guess,
                     Moose::FixedPoint::Value<2> & residual,
                     Moose::FixedPoint::Jacobian<2> & jacobian) {
    std::cout << guess(0) << ", " << guess(1) << '\n';

    residual(0) = guess(0) + guess(0) * guess(1) - 4;
    residual(1) = guess(0) + guess(1) - 3;

    jacobian(0, 0) = 1 + guess(1);
    jacobian(0, 1) = guess(0);
    jacobian(1, 0) = 1;
    jacobian(1, 1) = 1;
  };

  Moose::FixedPoint::Value<2> solution{1.98, 1.02};
  Moose::FixedPoint::nonlinearSolve<2>(solution, compute);
}
