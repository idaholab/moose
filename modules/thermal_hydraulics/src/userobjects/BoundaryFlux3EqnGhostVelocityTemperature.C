#include "BoundaryFlux3EqnGhostVelocityTemperature.h"
#include "THMIndices3Eqn.h"
#include "SinglePhaseFluidProperties.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", BoundaryFlux3EqnGhostVelocityTemperature);

InputParameters
BoundaryFlux3EqnGhostVelocityTemperature::validParams()
{
  InputParameters params = BoundaryFlux3EqnGhostBase::validParams();

  params.addClassDescription(
      "Computes a boundary flux from a specified velocity and temperature for the 1-D, "
      "1-phase, variable-area Euler equations using a ghost cell");

  params.addRequiredParam<Real>("vel", "Specified velocity");
  params.addRequiredParam<Real>("T", "Specified temperature");
  params.addParam<bool>("reversible", true, "True for reversible, false for pure inlet");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name of single-phase fluid properties user object");

  params.declareControllable("vel T");
  return params;
}

BoundaryFlux3EqnGhostVelocityTemperature::BoundaryFlux3EqnGhostVelocityTemperature(
    const InputParameters & parameters)
  : BoundaryFlux3EqnGhostBase(parameters),

    _vel(getParam<Real>("vel")),
    _T(getParam<Real>("T")),
    _reversible(getParam<bool>("reversible")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
}

std::vector<Real>
BoundaryFlux3EqnGhostVelocityTemperature::getGhostCellSolution(const std::vector<Real> & U) const
{
  const Real rhoA = U[THM3Eqn::CONS_VAR_RHOA];
  const Real rhouA = U[THM3Eqn::CONS_VAR_RHOUA];
  const Real rhoEA = U[THM3Eqn::CONS_VAR_RHOEA];
  const Real A = U[THM3Eqn::CONS_VAR_AREA];

  const Real rho = rhoA / A;
  std::vector<Real> U_ghost(THM3Eqn::N_CONS_VAR);
  if (!_reversible || THM::isInlet(_vel, _normal))
  {
    // Pressure is the only quantity coming from the interior
    const Real vel = rhouA / rhoA;
    const Real E = rhoEA / rhoA;
    const Real e = E - 0.5 * vel * vel;
    const Real p = _fp.p_from_v_e(1.0 / rho, e);

    const Real rho_b = _fp.rho_from_p_T(p, _T);
    const Real rhouA_b = rho_b * _vel * A;
    const Real e_b = _fp.e_from_p_rho(p, rho_b);
    const Real E_b = e_b + 0.5 * _vel * _vel;

    U_ghost[THM3Eqn::CONS_VAR_RHOA] = rho_b * A;
    U_ghost[THM3Eqn::CONS_VAR_RHOUA] = rhouA_b;
    U_ghost[THM3Eqn::CONS_VAR_RHOEA] = rho_b * E_b * A;
    U_ghost[THM3Eqn::CONS_VAR_AREA] = A;
  }
  else
  {
    U_ghost[THM3Eqn::CONS_VAR_RHOA] = rhoA;
    U_ghost[THM3Eqn::CONS_VAR_RHOUA] = rhoA * _vel;
    U_ghost[THM3Eqn::CONS_VAR_RHOEA] = rhoEA;
    U_ghost[THM3Eqn::CONS_VAR_AREA] = A;
  }

  return U_ghost;
}

DenseMatrix<Real>
BoundaryFlux3EqnGhostVelocityTemperature::getGhostCellSolutionJacobian(
    const std::vector<Real> & U) const
{
  const Real rhoA = U[THM3Eqn::CONS_VAR_RHOA];
  const Real rhouA = U[THM3Eqn::CONS_VAR_RHOUA];
  const Real rhoEA = U[THM3Eqn::CONS_VAR_RHOEA];
  const Real A = U[THM3Eqn::CONS_VAR_AREA];

  DenseMatrix<Real> J(THM3Eqn::N_EQ, THM3Eqn::N_EQ);
  if (!_reversible || THM::isInlet(_vel, _normal))
  {
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

    Real e_b, de_b_dp, de_b_drho_b;
    _fp.e_from_p_rho(p, rho_b, e_b, de_b_dp, de_b_drho_b);
    const Real de_b_drhoA = de_b_dp * dp_drhoA + de_b_drho_b * drho_b_drhoA;
    const Real de_b_drhouA = de_b_dp * dp_drhouA + de_b_drho_b * drho_b_drhouA;
    const Real de_b_drhoEA = de_b_dp * dp_drhoEA + de_b_drho_b * drho_b_drhoEA;

    Real E_b, dE_b_de_b, dE_b_dvel_b;
    THM::E_from_e_vel(e_b, _vel, E_b, dE_b_de_b, dE_b_dvel_b);
    const Real dE_b_drhoA = dE_b_de_b * de_b_drhoA;
    const Real dE_b_drhouA = dE_b_de_b * de_b_drhouA;
    const Real dE_b_drhoEA = dE_b_de_b * de_b_drhoEA;

    J(THM3Eqn::EQ_MASS, THM3Eqn::EQ_MASS) = drho_b_drhoA * A;
    J(THM3Eqn::EQ_MASS, THM3Eqn::EQ_MOMENTUM) = drho_b_drhouA * A;
    J(THM3Eqn::EQ_MASS, THM3Eqn::EQ_ENERGY) = drho_b_drhoEA * A;

    J(THM3Eqn::EQ_MOMENTUM, THM3Eqn::EQ_MASS) = drho_b_drhoA * _vel * A;
    J(THM3Eqn::EQ_MOMENTUM, THM3Eqn::EQ_MOMENTUM) = drho_b_drhouA * _vel * A;
    J(THM3Eqn::EQ_MOMENTUM, THM3Eqn::EQ_ENERGY) = drho_b_drhoEA * _vel * A;

    J(THM3Eqn::EQ_ENERGY, THM3Eqn::EQ_MASS) = (drho_b_drhoA * E_b + rho_b * dE_b_drhoA) * A;
    J(THM3Eqn::EQ_ENERGY, THM3Eqn::EQ_MOMENTUM) = (drho_b_drhouA * E_b + rho_b * dE_b_drhouA) * A;
    J(THM3Eqn::EQ_ENERGY, THM3Eqn::EQ_ENERGY) = (drho_b_drhoEA * E_b + rho_b * dE_b_drhoEA) * A;
  }
  else
  {
    J(THM3Eqn::EQ_MASS, THM3Eqn::EQ_MASS) = 1.;
    J(THM3Eqn::EQ_MOMENTUM, THM3Eqn::EQ_MASS) = _vel;
    J(THM3Eqn::EQ_ENERGY, THM3Eqn::EQ_ENERGY) = 1.;
  }

  return J;
}
