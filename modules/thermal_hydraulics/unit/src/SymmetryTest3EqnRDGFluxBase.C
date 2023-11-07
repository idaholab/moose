//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SymmetryTest3EqnRDGFluxBase.h"
#include "SinglePhaseFluidPropertiesTestUtils.h"
#include "THMIndicesVACE.h"

void
SymmetryTest3EqnRDGFluxBase::test()
{
  _flux = createFluxObject();

  std::set<unsigned int> flux_regions;

  unsigned int i_side = 0;
  const auto W_pairs = getPrimitiveSolutionPairs();
  for (const auto & W_pair : W_pairs)
  {
    const auto & WL = W_pair.first;
    const auto & WR = W_pair.second;

    const ADReal AL = 1.0;
    const ADReal AR = 1.5;

    const std::vector<ADReal> UL = computeConservativeSolution(WL, AL);
    const std::vector<ADReal> UR = computeConservativeSolution(WR, AR);

    const auto FLR = _flux->getFlux(i_side, 0, true, UL, UR, _nLR_dot_d);
    const auto FRL = _flux->getFlux(i_side, 0, false, UL, UR, _nLR_dot_d);
    i_side++;
    flux_regions.insert(_flux->getLastRegionIndex());

    const auto FRL_flipped = _flux->getFlux(i_side, 0, true, UR, UL, -_nLR_dot_d);
    const auto FLR_flipped = _flux->getFlux(i_side, 0, false, UR, UL, -_nLR_dot_d);
    i_side++;
    flux_regions.insert(_flux->getLastRegionIndex());

    for (unsigned int i = 0; i < THMVACE1D::N_FLUX_OUTPUTS; ++i)
    {
      REL_TEST(FLR[i], FLR_flipped[i], REL_TOL_CONSISTENCY);
      REL_TEST(FRL[i], FRL_flipped[i], REL_TOL_CONSISTENCY);
    }
  }

  // Check that all of the regions in the flux have been tested.
  EXPECT_TRUE(flux_regions.size() == _flux->getNumberOfRegions());
}
