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

  std::vector<Real> U(THM3Eqn::N_CONS_VAR, 0.0);
  U[THM3Eqn::CONS_VAR_RHOA] = rho * _A;
  U[THM3Eqn::CONS_VAR_RHOUA] = rho * vel * _A;
  U[THM3Eqn::CONS_VAR_RHOEA] = rho * E * _A;
  U[THM3Eqn::CONS_VAR_AREA] = _A;

  std::vector<Real> F_computed;
  _flux->calcFlux(U, U, _nLR_dot_d, F_computed);

  std::vector<Real> F_expected(THM3Eqn::N_EQ, 0.0);
  F_expected[THM3Eqn::EQ_MASS] = rho * vel * _A;
  F_expected[THM3Eqn::EQ_MOMENTUM] = (rho * vel * vel + p) * _A;
  F_expected[THM3Eqn::EQ_ENERGY] = vel * (rho * E + p) * _A;

  for (unsigned int i = 0; i < THM3Eqn::N_EQ; ++i)
    REL_TEST(F_computed[i], F_expected[i], REL_TOL_CONSISTENCY);
}
