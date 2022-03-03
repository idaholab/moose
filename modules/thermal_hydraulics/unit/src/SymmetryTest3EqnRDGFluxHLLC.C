//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SymmetryTest3EqnRDGFluxHLLC.h"

TEST_F(SymmetryTest3EqnRDGFluxHLLC, test) { test(); }

std::vector<std::pair<std::vector<ADReal>, std::vector<ADReal>>>
SymmetryTest3EqnRDGFluxHLLC::getPrimitiveSolutionPairs() const
{
  // sL < 0 < sM
  const std::vector<ADReal> W1{1e5, 300, 20.0};
  const std::vector<ADReal> W2{2e5, 310, 1.2};

  // sL > 0
  const std::vector<ADReal> W3{1e5, 300, 20};
  const std::vector<ADReal> W4{2e5, 310, 25};

  std::vector<std::pair<std::vector<ADReal>, std::vector<ADReal>>> W_pairs;
  W_pairs.push_back(std::pair<std::vector<ADReal>, std::vector<ADReal>>(W1, W2));
  W_pairs.push_back(std::pair<std::vector<ADReal>, std::vector<ADReal>>(W3, W4));

  return W_pairs;
}
