#include "SymmetryTest3EqnRDGFluxCentered.h"

TEST_F(SymmetryTest3EqnRDGFluxCentered, test) { test(); }

std::vector<std::pair<std::vector<Real>, std::vector<Real>>>
SymmetryTest3EqnRDGFluxCentered::getPrimitiveSolutionPairs() const
{
  const std::vector<Real> W1{1e5, 300, 1.5};
  const std::vector<Real> W2{2e5, 310, 1.2};

  std::vector<std::pair<std::vector<Real>, std::vector<Real>>> W_pairs;
  W_pairs.push_back(std::pair<std::vector<Real>, std::vector<Real>>(W1, W2));

  return W_pairs;
}
