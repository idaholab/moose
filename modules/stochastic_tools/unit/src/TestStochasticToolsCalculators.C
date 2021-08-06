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
  const std::vector<std::string> compute = {"mean", "min", "max", "sum", "stddev", "stderr", "ratio", "norm2"};
  const std::vector<Real> expect = {4, 1, 7, 28, 2.1602468994692869408, 0.81649658092772603446, 7, 11.832159566199232259};

  Parallel::Communicator comm;
  ParallelObject po(comm);
  std::vector<std::unique_ptr<StochasticTools::Calculator<std::vector<Real>, Real>>> calcs;
  for (const auto & stat : compute)
    calcs.push_back(StochasticTools::makeCalculator<std::vector<Real>, Real>(stat, po));

  for (unsigned int i = 0; i < calcs.size(); ++i)
    EXPECT_EQ(calcs[i]->compute(x, false), expect[i]);

  for (unsigned int i = 0; i < calcs.size(); ++i)
  {
    calcs[i]->initialize();
    for (const auto & val : x)
      calcs[i]->update(val);
    calcs[i]->finalize(false);
    EXPECT_EQ(calcs[i]->get(), expect[i]);
  }
}
