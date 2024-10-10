//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestNumericalFlux3EqnBase.h"
#include "THMIndicesVACE.h"

std::vector<ADReal>
TestNumericalFlux3EqnBase::computeConservativeSolution(const std::vector<ADReal> & W,
                                                       const ADReal & A) const
{
  const ADReal & p = W[0];
  const ADReal & T = W[1];
  const ADReal & vel = W[2];

  const ADReal rho = _fp.rho_from_p_T(p, T);
  const ADReal e = _fp.e_from_p_rho(p, rho);
  const ADReal E = e + 0.5 * vel * vel;

  std::vector<ADReal> U(THMVACE1D::N_FLUX_INPUTS, 0.0);
  U[THMVACE1D::RHOA] = rho * A;
  U[THMVACE1D::RHOUA] = rho * vel * A;
  U[THMVACE1D::RHOEA] = rho * E * A;
  U[THMVACE1D::AREA] = A;

  return U;
}

std::vector<ADReal>
TestNumericalFlux3EqnBase::computeFluxFromPrimitive(const std::vector<ADReal> & W,
                                                    const ADReal & A) const
{
  const ADReal & p = W[0];
  const ADReal & T = W[1];
  const ADReal & vel = W[2];

  const ADReal rho = _fp.rho_from_p_T(p, T);
  const ADReal e = _fp.e_from_p_rho(p, rho);
  const ADReal E = e + 0.5 * vel * vel;

  std::vector<ADReal> F(THMVACE1D::N_FLUX_OUTPUTS, 0.0);
  F[THMVACE1D::MASS] = rho * vel * A;
  F[THMVACE1D::MOMENTUM] = (rho * vel * vel + p) * A;
  F[THMVACE1D::ENERGY] = vel * (rho * E + p) * A;

  return F;
}

void
TestNumericalFlux3EqnBase::testSymmetry()
{
  _flux = createFluxObject();

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

    const auto FLR = _flux->getFlux(i_side, 0, true, UL, UR, 1.0);
    const auto FRL = _flux->getFlux(i_side, 0, false, UL, UR, 1.0);
    i_side++;
    flux_regions.insert(_flux->getLastRegionIndex());

    const auto FRL_flipped = _flux->getFlux(i_side, 0, true, UR, UL, -1.0);
    const auto FLR_flipped = _flux->getFlux(i_side, 0, false, UR, UL, -1.0);
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

void
TestNumericalFlux3EqnBase::testConsistency()
{
  _flux = createFluxObject();

  unsigned int i_side = 0;
  const auto W_list = getPrimitiveSolutionsConsistencyTest();
  for (const auto & W : W_list)
  {
    const ADReal A = 2.0;

    const auto U = computeConservativeSolution(W, A);
    const auto F_expected = computeFluxFromPrimitive(W, A);

    const auto & FL_computed_pos = _flux->getFlux(i_side, 0, true, U, U, 1.0);
    const auto & FR_computed_pos = _flux->getFlux(i_side, 0, false, U, U, 1.0);
    i_side++;

    const auto & FL_computed_neg = _flux->getFlux(i_side, 0, true, U, U, -1.0);
    const auto & FR_computed_neg = _flux->getFlux(i_side, 0, false, U, U, -1.0);
    i_side++;

    for (unsigned int i = 0; i < THMVACE1D::N_FLUX_OUTPUTS; ++i)
    {
      REL_TEST(FL_computed_pos[i], F_expected[i], REL_TOL_CONSISTENCY);
      REL_TEST(FR_computed_pos[i], F_expected[i], REL_TOL_CONSISTENCY);

      REL_TEST(FL_computed_neg[i], F_expected[i], REL_TOL_CONSISTENCY);
      REL_TEST(FR_computed_neg[i], F_expected[i], REL_TOL_CONSISTENCY);
    }
  }
}
