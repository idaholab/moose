#include "SymmetryTest3EqnRDGFluxHLLC.h"

TEST_F(SymmetryTest3EqnRDGFluxHLLC, test) { test(); }

std::vector<std::pair<std::vector<Real>, std::vector<Real>>>
SymmetryTest3EqnRDGFluxHLLC::getPrimitiveSolutionPairs() const
{
  // sL < 0 < sM
  const std::vector<Real> W1{1e5, 300, 20.0};
  const std::vector<Real> W2{2e5, 310, 1.2};

  // sL > 0
  const std::vector<Real> W3{1e5, 300, 20};
  const std::vector<Real> W4{2e5, 310, 25};

  std::vector<std::pair<std::vector<Real>, std::vector<Real>>> W_pairs;
  W_pairs.push_back(std::pair<std::vector<Real>, std::vector<Real>>(W1, W2));
  W_pairs.push_back(std::pair<std::vector<Real>, std::vector<Real>>(W3, W4));

  return W_pairs;
}
