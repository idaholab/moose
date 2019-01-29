#include "BoundaryFlux3EqnGhostStagnationPressureTemperature.h"
#include "SinglePhaseFluidProperties.h"
#include "RELAP7Indices3Eqn.h"
#include "Numerics.h"

registerMooseObject("RELAP7App", BoundaryFlux3EqnGhostStagnationPressureTemperature);

template <>
InputParameters
validParams<BoundaryFlux3EqnGhostStagnationPressureTemperature>()
{
  InputParameters params = validParams<BoundaryFlux3EqnGhostBase>();

  params.addClassDescription("Computes boundary flux from a specified stagnation pressure and "
                             "temperature for the 1-D, 1-phase, variable-area Euler equations");

  params.addRequiredParam<Real>("p0", "Stagnation pressure");
  params.addRequiredParam<Real>("T0", "Stagnation temperature");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name of fluid properties user object");

  return params;
}

BoundaryFlux3EqnGhostStagnationPressureTemperature::
    BoundaryFlux3EqnGhostStagnationPressureTemperature(const InputParameters & parameters)
  : BoundaryFlux3EqnGhostBase(parameters),

    _p0(getParam<Real>("p0")),
    _T0(getParam<Real>("T0")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
}

std::vector<Real>
BoundaryFlux3EqnGhostStagnationPressureTemperature::getGhostCellSolution(
    const std::vector<Real> & U) const
{
  const Real rhoA = U[RELAP73Eqn::CONS_VAR_RHOA];
  const Real rhouA = U[RELAP73Eqn::CONS_VAR_RHOUA];
  const Real A = U[RELAP73Eqn::CONS_VAR_AREA];

  const Real vel = rhouA / rhoA;

  // compute stagnation quantities
  const Real rho0 = _fp.rho_from_p_T(_p0, _T0);
  const Real e0 = _fp.e_from_p_rho(_p0, rho0);
  const Real v0 = 1.0 / rho0;
  const Real h0 = _fp.h_from_p_T(_p0, _T0);
  const Real s0 = _fp.s_from_v_e(v0, e0);

  // compute static quantities
  const Real h = h0 - 0.5 * vel * vel;
  const Real s = s0;
  const Real p = _fp.p_from_h_s(h, s);
  const Real rho = _fp.rho_from_p_s(p, s);
  const Real e = _fp.e_from_p_rho(p, rho);
  const Real E = e + 0.5 * vel * vel;

  std::vector<Real> U_ghost(RELAP73Eqn::N_CONS_VAR);
  U_ghost[RELAP73Eqn::CONS_VAR_RHOA] = rho * A;
  U_ghost[RELAP73Eqn::CONS_VAR_RHOUA] = rho * vel * A;
  U_ghost[RELAP73Eqn::CONS_VAR_RHOEA] = rho * E * A;
  U_ghost[RELAP73Eqn::CONS_VAR_AREA] = A;

  return U_ghost;
}

DenseMatrix<Real>
BoundaryFlux3EqnGhostStagnationPressureTemperature::getGhostCellSolutionJacobian(
    const std::vector<Real> & U) const
{
  const Real rhoA = U[RELAP73Eqn::CONS_VAR_RHOA];
  const Real rhouA = U[RELAP73Eqn::CONS_VAR_RHOUA];
  const Real A = U[RELAP73Eqn::CONS_VAR_AREA];

  Real vel, dvel_drhoA, dvel_drhouA;
  THM::vel_from_arhoA_arhouA(rhoA, rhouA, vel, dvel_drhoA, dvel_drhouA);

  // compute stagnation quantities
  const Real rho0 = _fp.rho_from_p_T(_p0, _T0);
  const Real e0 = _fp.e_from_p_rho(_p0, rho0);
  const Real v0 = 1.0 / rho0;
  const Real h0 = _fp.h_from_p_T(_p0, _T0);
  const Real s0 = _fp.s_from_v_e(v0, e0);

  // compute static quantities

  const Real h = h0 - 0.5 * vel * vel;
  const Real dh_drhoA = -vel * dvel_drhoA;
  const Real dh_drhouA = -vel * dvel_drhouA;

  const Real s = s0;

  Real p, dp_dh, dp_ds;
  _fp.p_from_h_s(h, s, p, dp_dh, dp_ds);
  const Real dp_drhoA = dp_dh * dh_drhoA;
  const Real dp_drhouA = dp_dh * dh_drhouA;

  Real rho, drho_dp, drho_ds;
  _fp.rho_from_p_s(p, s, rho, drho_dp, drho_ds);
  const Real drho_drhoA = drho_dp * dp_drhoA;
  const Real drho_drhouA = drho_dp * dp_drhouA;

  Real e, de_dp, de_drho;
  _fp.e_from_p_rho(p, rho, e, de_dp, de_drho);
  const Real de_drhoA = de_dp * dp_drhoA + de_drho * drho_drhoA;
  const Real de_drhouA = de_dp * dp_drhouA + de_drho * drho_drhouA;

  const Real E = e + 0.5 * vel * vel;
  const Real dE_drhoA = de_drhoA + vel * dvel_drhoA;
  const Real dE_drhouA = de_drhouA + vel * dvel_drhouA;

  DenseMatrix<Real> J(RELAP73Eqn::N_EQ, RELAP73Eqn::N_EQ);

  J(RELAP73Eqn::EQ_MASS, RELAP73Eqn::EQ_MASS) = drho_drhoA * A;
  J(RELAP73Eqn::EQ_MASS, RELAP73Eqn::EQ_MOMENTUM) = drho_drhouA * A;

  J(RELAP73Eqn::EQ_MOMENTUM, RELAP73Eqn::EQ_MASS) = (drho_drhoA * vel + rho * dvel_drhoA) * A;
  J(RELAP73Eqn::EQ_MOMENTUM, RELAP73Eqn::EQ_MOMENTUM) = (drho_drhouA * vel + rho * dvel_drhouA) * A;

  J(RELAP73Eqn::EQ_ENERGY, RELAP73Eqn::EQ_MASS) = (drho_drhoA * E + rho * dE_drhoA) * A;
  J(RELAP73Eqn::EQ_ENERGY, RELAP73Eqn::EQ_MOMENTUM) = (drho_drhouA * E + rho * dE_drhouA) * A;

  return J;
}
