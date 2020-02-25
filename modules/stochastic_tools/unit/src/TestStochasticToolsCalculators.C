//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "gtest/gtest.h"
#include <vector>
#include "Calculators.h"
#include "libmesh/communicator.h"
#include "libmesh/parallel_object.h"

using namespace StochasticTools;

TEST(StochasticTools, Calculators)
{
  const std::vector<Real> x = {6, 1, 7, 3, 4, 5, 2};
  Parallel::Communicator comm;
  ParallelObject po(comm);

  {
    Mean calc(po);
    EXPECT_EQ(calc.compute(x, false), 4);
  }

  {
    Min calc(po);
    EXPECT_EQ(calc.compute(x, false), 1);
  }

  {
    Max calc(po);
    EXPECT_EQ(calc.compute(x, false), 7);
  }

  {
    Sum calc(po);
    EXPECT_EQ(calc.compute(x, false), 28);
  }

  {
    StdDev calc(po);
    EXPECT_EQ(calc.compute(x, false), 2.1602468994692869408);
  }

  {
    StdErr calc(po);
    EXPECT_EQ(calc.compute(x, false), 0.81649658092772603446);
  }

  {
    Ratio calc(po);
    EXPECT_EQ(calc.compute(x, false), 7);
  }

  {
    L2Norm calc(po);
    EXPECT_EQ(calc.compute(x, false), 11.832159566199232259);
  }
}
