#include "BoundaryFlux3EqnGhostMassFlowRateTemperature.h"
#include "THMIndices3Eqn.h"
#include "SinglePhaseFluidProperties.h"
#include "Numerics.h"

registerMooseObject("THMApp", BoundaryFlux3EqnGhostMassFlowRateTemperature);

template <>
InputParameters
validParams<BoundaryFlux3EqnGhostMassFlowRateTemperature>()
{
  InputParameters params = validParams<BoundaryFlux3EqnGhostBase>();

  params.addClassDescription(
      "Computes a boundary flux from a specified mass flow rate and temperature for the 1-D, "
      "1-phase, variable-area Euler equations using a ghost cell");

  params.addRequiredParam<Real>("mass_flow_rate", "Specified mass flow rate");
  params.addRequiredParam<Real>("T", "Specified temperature");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name of single-phase fluid properties user object");

  params.declareControllable("mass_flow_rate T");
  return params;
}

BoundaryFlux3EqnGhostMassFlowRateTemperature::BoundaryFlux3EqnGhostMassFlowRateTemperature(
    const InputParameters & parameters)
  : BoundaryFlux3EqnGhostBase(parameters),

    _rhouA(getParam<Real>("mass_flow_rate")),
    _T(getParam<Real>("T")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
}

std::vector<Real>
BoundaryFlux3EqnGhostMassFlowRateTemperature::getGhostCellSolution(
    const std::vector<Real> & U) const
{
  const Real rhoA = U[THM3Eqn::CONS_VAR_RHOA];
  const Real rhouA = U[THM3Eqn::CONS_VAR_RHOUA];
  const Real rhoEA = U[THM3Eqn::CONS_VAR_RHOEA];
  const Real A = U[THM3Eqn::CONS_VAR_AREA];

  // Pressure is the only quantity coming from the interior
  const Real rho = rhoA / A;
  const Real vel = rhouA / rhoA;
  const Real E = rhoEA / rhoA;
  const Real e = E - 0.5 * vel * vel;
  const Real p = _fp.p_from_v_e(1.0 / rho, e);

  const Real rho_b = _fp.rho_from_p_T(p, _T);
  const Real vel_b = _rhouA / (rho_b * A);
  const Real e_b = _fp.e_from_p_rho(p, rho_b);
  const Real E_b = e_b + 0.5 * vel_b * vel_b;

  std::vector<Real> U_ghost(THM3Eqn::N_CONS_VAR);
  U_ghost[THM3Eqn::CONS_VAR_RHOA] = rho_b * A;
  U_ghost[THM3Eqn::CONS_VAR_RHOUA] = _rhouA;
  U_ghost[THM3Eqn::CONS_VAR_RHOEA] = rho_b * E_b * A;
  U_ghost[THM3Eqn::CONS_VAR_AREA] = A;

  return U_ghost;
}

DenseMatrix<Real>
BoundaryFlux3EqnGhostMassFlowRateTemperature::getGhostCellSolutionJacobian(
    const std::vector<Real> & U) const
{
  const Real rhoA = U[THM3Eqn::CONS_VAR_RHOA];
  const Real rhouA = U[THM3Eqn::CONS_VAR_RHOUA];
  const Real rhoEA = U[THM3Eqn::CONS_VAR_RHOEA];
  const Real A = U[THM3Eqn::CONS_VAR_AREA];

  // Compute pressure from the interior solution
  const Real rho = rhoA / A;
  const Real drho_drhoA = 1.0 / A;

  Real v, dv_drho;
  THM::v_from_rho(rho, v, dv_drho);
  const Real dv_drhoA = dv_drho * drho_drhoA;

  Real vel, dvel_drhoA, dvel_drhouA;
  THM::vel_from_arhoA_arhouA(rhoA, rhouA, vel, dvel_drhoA, dvel_drhouA);

  Real E, dE_drhoA, dE_drhoEA;
  THM::E_from_arhoA_arhoEA(rhoA, rhoEA, E, dE_drhoA, dE_drhoEA);

  Real e, de_dE, de_dvel;
  THM::e_from_E_vel(E, vel, e, de_dE, de_dvel);
  const Real de_drhoA = de_dE * dE_drhoA + de_dvel * dvel_drhoA;
  const Real de_drhouA = de_dvel * dvel_drhouA;
  const Real de_drhoEA = de_dE * dE_drhoEA;

  Real p, dp_dv, dp_de;
  _fp.p_from_v_e(v, e, p, dp_dv, dp_de);
  const Real dp_drhoA = dp_dv * dv_drhoA + dp_de * de_drhoA;
  const Real dp_drhouA = dp_de * de_drhouA;
  const Real dp_drhoEA = dp_de * de_drhoEA;

  // Compute boundary quantities
  Real rho_b, drho_b_dp, drho_b_dT;
  _fp.rho_from_p_T(p, _T, rho_b, drho_b_dp, drho_b_dT);
  const Real drho_b_drhoA = drho_b_dp * dp_drhoA;
  const Real drho_b_drhouA = drho_b_dp * dp_drhouA;
  const Real drho_b_drhoEA = drho_b_dp * dp_drhoEA;

  Real vel_b, dvel_b_drhobA, dvel_b_drhouA_b;
  THM::vel_from_arhoA_arhouA(rho_b * A, _rhouA, vel_b, dvel_b_drhobA, dvel_b_drhouA_b);
  const Real dvel_b_drhoA = dvel_b_drhobA * drho_b_drhoA * A;
  const Real dvel_b_drhouA = dvel_b_drhobA * drho_b_drhouA * A;
  const Real dvel_b_drhoEA = dvel_b_drhobA * drho_b_drhoEA * A;

  Real e_b, de_b_dp, de_b_drho_b;
  _fp.e_from_p_rho(p, rho_b, e_b, de_b_dp, de_b_drho_b);
  const Real de_b_drhoA = de_b_dp * dp_drhoA + de_b_drho_b * drho_b_drhoA;
  const Real de_b_drhouA = de_b_dp * dp_drhouA + de_b_drho_b * drho_b_drhouA;
  const Real de_b_drhoEA = de_b_dp * dp_drhoEA + de_b_drho_b * drho_b_drhoEA;

  Real E_b, dE_b_de_b, dE_b_dvel_b;
  THM::E_from_e_vel(e_b, vel_b, E_b, dE_b_de_b, dE_b_dvel_b);
  const Real dE_b_drhoA = dE_b_de_b * de_b_drhoA + dE_b_dvel_b * dvel_b_drhoA;
  const Real dE_b_drhouA = dE_b_de_b * de_b_drhouA + dE_b_dvel_b * dvel_b_drhouA;
  const Real dE_b_drhoEA = dE_b_de_b * de_b_drhoEA + dE_b_dvel_b * dvel_b_drhoEA;

  DenseMatrix<Real> J(THM3Eqn::N_EQ, THM3Eqn::N_EQ);

  J(THM3Eqn::EQ_MASS, THM3Eqn::EQ_MASS) = drho_b_drhoA * A;
  J(THM3Eqn::EQ_MASS, THM3Eqn::EQ_MOMENTUM) = drho_b_drhouA * A;
  J(THM3Eqn::EQ_MASS, THM3Eqn::EQ_ENERGY) = drho_b_drhoEA * A;

  J(THM3Eqn::EQ_ENERGY, THM3Eqn::EQ_MASS) = (drho_b_drhoA * E_b + rho_b * dE_b_drhoA) * A;
  J(THM3Eqn::EQ_ENERGY, THM3Eqn::EQ_MOMENTUM) = (drho_b_drhouA * E_b + rho_b * dE_b_drhouA) * A;
  J(THM3Eqn::EQ_ENERGY, THM3Eqn::EQ_ENERGY) = (drho_b_drhoEA * E_b + rho_b * dE_b_drhoEA) * A;

  return J;
}
