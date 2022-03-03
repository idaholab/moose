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
#include "THMIndices3Eqn.h"

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

  std::vector<ADReal> U(THM3Eqn::N_CONS_VAR, 0.0);
  U[THM3Eqn::CONS_VAR_RHOA] = rho * A;
  U[THM3Eqn::CONS_VAR_RHOUA] = rho * vel * A;
  U[THM3Eqn::CONS_VAR_RHOEA] = rho * E * A;
  U[THM3Eqn::CONS_VAR_AREA] = A;

  std::vector<ADReal> FL_computed, FR_computed;
  _flux->calcFlux(U, U, _nLR_dot_d, FL_computed, FR_computed);

  std::vector<ADReal> F_expected(THM3Eqn::N_EQ, 0.0);
  F_expected[THM3Eqn::EQ_MASS] = rho * vel * A;
  F_expected[THM3Eqn::EQ_MOMENTUM] = (rho * vel * vel + p) * A;
  F_expected[THM3Eqn::EQ_ENERGY] = vel * (rho * E + p) * A;

  for (unsigned int i = 0; i < THM3Eqn::N_EQ; ++i)
  {
    REL_TEST(FL_computed[i], F_expected[i], REL_TOL_CONSISTENCY);
    REL_TEST(FR_computed[i], F_expected[i], REL_TOL_CONSISTENCY);
  }
}
