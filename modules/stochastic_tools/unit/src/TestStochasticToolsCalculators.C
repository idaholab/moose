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
#include "VectorCalculators.h"
#include "libmesh/communicator.h"
#include "libmesh/parallel_object.h"

using namespace libMesh;
using namespace StochasticTools;

template <typename InType, typename OutType>
std::pair<std::vector<OutType>, std::vector<OutType>>
calculate(const InType & x, const std::vector<std::string> & compute)
{
  Parallel::Communicator comm;
  ParallelObject po(comm);
  std::vector<std::unique_ptr<StochasticTools::Calculator<InType, OutType>>> calcs;
  for (const auto & stat : compute)
    calcs.push_back(StochasticTools::makeCalculator<InType, OutType>(stat, po));

  std::vector<OutType> result1;
  for (unsigned int i = 0; i < calcs.size(); ++i)
    result1.push_back(calcs[i]->compute(x, false));

  std::vector<OutType> result2;
  for (unsigned int i = 0; i < calcs.size(); ++i)
  {
    calcs[i]->initializeCalculator();
    for (const auto & val : x)
      calcs[i]->updateCalculator(val);
    calcs[i]->finalizeCalculator(false);
    result2.push_back(calcs[i]->getValue());
  }

  return {result1, result2};
}

TEST(StochasticTools, Calculators)
{
  const std::vector<std::string> compute = {
      "mean", "min", "max", "sum", "stddev", "stderr", "ratio", "norm2", "median"};

  {
    const std::vector<Real> x = {6, 1, 7, 3, 4, 5, 2};
    const std::vector<Real> expect = {
        4, 1, 7, 28, 2.1602468994692869408, 0.81649658092772603446, 7, 11.832159566199232259, 4};

    auto result = calculate<std::vector<Real>, Real>(x, compute);
    for (unsigned int i = 0; i < expect.size(); ++i)
    {
      EXPECT_EQ(result.first[i], expect[i]);
      EXPECT_EQ(result.second[i], expect[i]);
    }
  }

  {
    const std::vector<int> x = {6, 1, 7, 3, 4, 5, 2};
    const std::vector<Real> expect = {
        4, 1, 7, 28, 2.1602468994692869408, 0.81649658092772603446, 7, 11.832159566199232259, 4};

    auto result = calculate<std::vector<int>, Real>(x, compute);
    for (unsigned int i = 0; i < expect.size(); ++i)
    {
      EXPECT_EQ(result.first[i], expect[i]);
      EXPECT_EQ(result.second[i], expect[i]);
    }
  }

  {
    const std::vector<std::vector<Real>> x = {{6, 1, 7, 3, 4, 5, 2},
                                              {1, 7, 3, 4, 5, 2, 6},
                                              {7, 3, 4, 5, 2, 6, 1},
                                              {3, 4, 5, 2, 6, 1, 7},
                                              {4, 5, 2, 6, 1, 7, 3},
                                              {5, 2, 6, 1, 7, 3, 4},
                                              {2, 6, 1, 7, 3, 4, 5}};
    const std::vector<Real> expect = {
        4, 1, 7, 28, 2.1602468994692869408, 0.81649658092772603446, 7, 11.832159566199232259, 4};

    auto result = calculate<std::vector<std::vector<Real>>, std::vector<Real>>(x, compute);
    for (unsigned int i = 0; i < expect.size(); ++i)
      for (unsigned int j = 0; j < x[0].size(); ++j)
      {
        EXPECT_EQ(result.first[i][j], expect[i]);
        EXPECT_EQ(result.second[i][j], expect[i]);
      }
  }
}
