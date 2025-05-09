//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestNumericalFlux1D.h"
#include "FluidPropertiesTestUtils.h"

TestNumericalFlux1D::TestNumericalFlux1D()
  : MooseObjectUnitTest("ThermalHydraulicsApp")
{
}

void
TestNumericalFlux1D::testSymmetry()
{
  const auto & flux = createFluxObject();

  std::set<unsigned int> flux_regions;

  unsigned int i_side = 0;
  const auto W_pairs = getPrimitiveSolutionsSymmetryTest();
  for (const auto & W_pair : W_pairs)
  {
    const auto & WL = W_pair.first;
    const auto & WR = W_pair.second;

    const ADReal AL = 1.0;
    const ADReal AR = 1.5;

    const std::vector<ADReal> UL = computeConservativeSolution(WL, AL);
    const std::vector<ADReal> UR = computeConservativeSolution(WR, AR);

    const auto FLR = flux.getFlux(i_side, 0, true, UL, UR, 1.0);
    const auto FRL = flux.getFlux(i_side, 0, false, UL, UR, 1.0);
    i_side++;
    flux_regions.insert(flux.getLastRegionIndex());

    const auto FRL_flipped = flux.getFlux(i_side, 0, true, UR, UL, -1.0);
    const auto FLR_flipped = flux.getFlux(i_side, 0, false, UR, UL, -1.0);
    i_side++;
    flux_regions.insert(flux.getLastRegionIndex());

    for (unsigned int i = 0; i < FLR.size(); ++i)
    {
      REL_TEST(FLR[i], FLR_flipped[i], REL_TOL_CONSISTENCY);
      REL_TEST(FRL[i], FRL_flipped[i], REL_TOL_CONSISTENCY);
    }
  }

  // Check that all of the regions in the flux have been tested.
  EXPECT_TRUE(flux_regions.size() == flux.getNumberOfRegions());
}

void
TestNumericalFlux1D::testConsistency()
{
  const auto & flux = createFluxObject();

  unsigned int i_side = 0;
  const auto W_list = getPrimitiveSolutionsConsistencyTest();
  for (const auto & W : W_list)
  {
    const ADReal A = 2.0;

    const auto U = computeConservativeSolution(W, A);
    const auto F_expected = computeFluxFromPrimitive(W, A);

    const auto & FL_computed_pos = flux.getFlux(i_side, 0, true, U, U, 1.0);
    const auto & FR_computed_pos = flux.getFlux(i_side, 0, false, U, U, 1.0);
    i_side++;

    const auto & FL_computed_neg = flux.getFlux(i_side, 0, true, U, U, -1.0);
    const auto & FR_computed_neg = flux.getFlux(i_side, 0, false, U, U, -1.0);
    i_side++;

    for (unsigned int i = 0; i < FL_computed_pos.size(); ++i)
    {
      REL_TEST(FL_computed_pos[i], F_expected[i], REL_TOL_CONSISTENCY);
      REL_TEST(FR_computed_pos[i], F_expected[i], REL_TOL_CONSISTENCY);

      REL_TEST(FL_computed_neg[i], F_expected[i], REL_TOL_CONSISTENCY);
      REL_TEST(FR_computed_neg[i], F_expected[i], REL_TOL_CONSISTENCY);
    }
  }
}
