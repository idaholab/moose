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

  const Real rho = _fp.rho_from_p_T(p, T);
  const Real e = _fp.e_from_p_rho(p, rho);
  const Real E = e + 0.5 * vel * vel;

  const Real A = 2.0;

  std::vector<Real> U(THM3Eqn::N_CONS_VAR, 0.0);
  U[THM3Eqn::CONS_VAR_RHOA] = rho * A;
  U[THM3Eqn::CONS_VAR_RHOUA] = rho * vel * A;
  U[THM3Eqn::CONS_VAR_RHOEA] = rho * E * A;
  U[THM3Eqn::CONS_VAR_AREA] = A;

  std::vector<Real> FL_computed, FR_computed;
  _flux->calcFlux(U, U, _nLR_dot_d, FL_computed, FR_computed);

  std::vector<Real> F_expected(THM3Eqn::N_EQ, 0.0);
  F_expected[THM3Eqn::EQ_MASS] = rho * vel * A;
  F_expected[THM3Eqn::EQ_MOMENTUM] = (rho * vel * vel + p) * A;
  F_expected[THM3Eqn::EQ_ENERGY] = vel * (rho * E + p) * A;

  for (unsigned int i = 0; i < THM3Eqn::N_EQ; ++i)
  {
    REL_TEST(FL_computed[i], F_expected[i], REL_TOL_CONSISTENCY);
    REL_TEST(FR_computed[i], F_expected[i], REL_TOL_CONSISTENCY);
  }
}
