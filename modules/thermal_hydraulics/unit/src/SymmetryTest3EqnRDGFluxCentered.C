//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SymmetryTest3EqnRDGFluxCentered.h"

TEST_F(SymmetryTest3EqnRDGFluxCentered, test) { test(); }

std::vector<std::pair<std::vector<ADReal>, std::vector<ADReal>>>
SymmetryTest3EqnRDGFluxCentered::getPrimitiveSolutionPairs() const
{
  const std::vector<ADReal> W1{1e5, 300, 1.5};
  const std::vector<ADReal> W2{2e5, 310, 1.2};

  std::vector<std::pair<std::vector<ADReal>, std::vector<ADReal>>> W_pairs;
  W_pairs.push_back(std::pair<std::vector<ADReal>, std::vector<ADReal>>(W1, W2));

  return W_pairs;
}
