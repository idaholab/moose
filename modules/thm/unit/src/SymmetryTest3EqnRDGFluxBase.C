#include "SymmetryTest3EqnRDGFluxBase.h"
#include "SinglePhaseFluidPropertiesTestUtils.h"
#include "THMIndices3Eqn.h"

void
SymmetryTest3EqnRDGFluxBase::test()
{
  _flux = createFluxObject();

  std::set<unsigned int> flux_regions;

  const auto W_pairs = getPrimitiveSolutionPairs();
  for (const auto & W_pair : W_pairs)
  {
    const auto & WL = W_pair.first;
    const auto & WR = W_pair.second;

    const std::vector<Real> UL = computeConservativeSolution(WL);
    const std::vector<Real> UR = computeConservativeSolution(WR);

    std::vector<Real> FLR;
    _flux->calcFlux(UL, UR, _nLR_dot_d, FLR);
    flux_regions.insert(_flux->getLastRegionIndex());

    std::vector<Real> FRL;
    _flux->calcFlux(UR, UL, -_nLR_dot_d, FRL);
    flux_regions.insert(_flux->getLastRegionIndex());

    for (unsigned int i = 0; i < THM3Eqn::N_EQ; ++i)
      REL_TEST(FLR[i], FRL[i], REL_TOL_CONSISTENCY);
  }

  // Check that all of the regions in the flux have been tested.
  EXPECT_TRUE(flux_regions.size() == _flux->getNumberOfRegions());
}
