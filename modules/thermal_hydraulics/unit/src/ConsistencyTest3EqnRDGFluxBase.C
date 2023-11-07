//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConsistencyTest3EqnRDGFluxBase.h"
#include "SinglePhaseFluidPropertiesTestUtils.h"
#include "THMIndicesVACE.h"

void
ConsistencyTest3EqnRDGFluxBase::test()
{
  _flux = createFluxObject();

  const Real p = 1e5;
  const Real T = 300.0;
  const Real vel = 1.5;

  const ADReal rho = _fp.rho_from_p_T(p, T);
  const ADReal e = _fp.e_from_p_rho(p, rho);
  const ADReal E = e + 0.5 * vel * vel;

  const ADReal A = 2.0;

  std::vector<ADReal> U(THMVACE1D::N_FLUX_INPUTS, 0.0);
  U[THMVACE1D::RHOA] = rho * A;
  U[THMVACE1D::RHOUA] = rho * vel * A;
  U[THMVACE1D::RHOEA] = rho * E * A;
  U[THMVACE1D::AREA] = A;

  const auto & FL_computed = _flux->getFlux(0, 0, true, U, U, _nLR_dot_d);
  const auto & FR_computed = _flux->getFlux(0, 0, false, U, U, _nLR_dot_d);

  std::vector<ADReal> F_expected(THMVACE1D::N_FLUX_OUTPUTS, 0.0);
  F_expected[THMVACE1D::MASS] = rho * vel * A;
  F_expected[THMVACE1D::MOMENTUM] = (rho * vel * vel + p) * A;
  F_expected[THMVACE1D::ENERGY] = vel * (rho * E + p) * A;

  for (unsigned int i = 0; i < THMVACE1D::N_FLUX_OUTPUTS; ++i)
  {
    REL_TEST(FL_computed[i], F_expected[i], REL_TOL_CONSISTENCY);
    REL_TEST(FR_computed[i], F_expected[i], REL_TOL_CONSISTENCY);
  }
}
