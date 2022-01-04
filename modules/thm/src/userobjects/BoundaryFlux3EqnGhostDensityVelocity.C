#include "BoundaryFlux3EqnGhostDensityVelocity.h"
#include "SinglePhaseFluidProperties.h"
#include "Numerics.h"
#include "THMIndices3Eqn.h"

registerMooseObject("THMApp", BoundaryFlux3EqnGhostDensityVelocity);

InputParameters
BoundaryFlux3EqnGhostDensityVelocity::validParams()
{
  InputParameters params = BoundaryFlux3EqnGhostBase::validParams();

  params.addClassDescription("Computes boundary flux from density and velocity for the 3-equation "
                             "model using a ghost cell approach.");

  params.addRequiredParam<Real>("rho", "Density");
  params.addRequiredParam<Real>("vel", "Velocity");
  params.addParam<bool>("reversible", true, "True for reversible, false for pure inlet");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "1-phase fluid properties user object name");

  params.declareControllable("rho vel");

  return params;
}

BoundaryFlux3EqnGhostDensityVelocity::BoundaryFlux3EqnGhostDensityVelocity(
    const InputParameters & parameters)
  : BoundaryFlux3EqnGhostBase(parameters),

    _rho(getParam<Real>("rho")),
    _vel(getParam<Real>("vel")),
    _reversible(getParam<bool>("reversible")),

    _fp(getUserObjectByName<SinglePhaseFluidProperties>(
        getParam<UserObjectName>("fluid_properties")))
{
}

std::vector<Real>
BoundaryFlux3EqnGhostDensityVelocity::getGhostCellSolution(
    const std::vector<Real> & U_interior) const
{
  const Real rhoA = U_interior[THM3Eqn::CONS_VAR_RHOA];
  const Real rhouA = U_interior[THM3Eqn::CONS_VAR_RHOUA];
  const Real rhoEA = U_interior[THM3Eqn::CONS_VAR_RHOEA];
  const Real A = U_interior[THM3Eqn::CONS_VAR_AREA];

  std::vector<Real> U_ghost(THM3Eqn::N_CONS_VAR);
  if (!_reversible || THM::isInlet(_vel, _normal))
  {
    // Get the pressure from the interior solution

    const Real rho = rhoA / A;
    const Real vel = rhouA / rhoA;
    const Real E = rhoEA / rhoA;
    const Real e = E - 0.5 * vel * vel;
    const Real p = _fp.p_from_v_e(1.0 / rho, e);

    // Compute remaining boundary quantities

    const Real e_b = _fp.e_from_p_rho(p, _rho);
    const Real E_b = e_b + 0.5 * _vel * _vel;

    // compute ghost solution
    U_ghost[THM3Eqn::CONS_VAR_RHOA] = _rho * A;
    U_ghost[THM3Eqn::CONS_VAR_RHOUA] = _rho * _vel * A;
    U_ghost[THM3Eqn::CONS_VAR_RHOEA] = _rho * E_b * A;
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
BoundaryFlux3EqnGhostDensityVelocity::getGhostCellSolutionJacobian(
    const std::vector<Real> & U_interior) const
{
  const Real rhoA = U_interior[THM3Eqn::CONS_VAR_RHOA];
  const Real rhouA = U_interior[THM3Eqn::CONS_VAR_RHOUA];
  const Real rhoEA = U_interior[THM3Eqn::CONS_VAR_RHOEA];
  const Real A = U_interior[THM3Eqn::CONS_VAR_AREA];

  DenseMatrix<Real> J(THM3Eqn::N_EQ, THM3Eqn::N_EQ);
  if (!_reversible || THM::isInlet(_vel, _normal))
  {
    // Get the pressures from the interior solution

    Real rho = rhoA / A;
    Real drho_drhoA = 1.0 / A;

    Real v, dv_drho;
    THM::v_from_rho(rho, v, dv_drho);
    const Real dv_drhoA = dv_drho * drho_drhoA;

    const Real vel = rhouA / rhoA;

    const Real E = rhoEA / rhoA;
    const Real e = E - 0.5 * vel * vel;
    const Real de_drhoA = THM::de_darhoA(rhoA, rhouA, rhoEA);
    const Real de_drhouA = THM::de_darhouA(rhoA, rhouA);
    const Real de_drhoEA = THM::de_darhoEA(rhoA);

    Real p, dp_dv, dp_de;
    _fp.p_from_v_e(v, e, p, dp_dv, dp_de);
    const Real dp_drhoA = dp_dv * dv_drhoA + dp_de * de_drhoA;
    const Real dp_drhouA = dp_de * de_drhouA;
    const Real dp_drhoEA = dp_de * de_drhoEA;

    // Compute remaining boundary quantities

    const Real rhoA_b = _rho * A;

    Real e_b, de_b_dp, de_b_drho_b;
    _fp.e_from_p_rho(p, _rho, e_b, de_b_dp, de_b_drho_b);
    const Real de_b_drhoA = de_b_dp * dp_drhoA;
    const Real de_b_drhouA = de_b_dp * dp_drhouA;
    const Real de_b_drhoEA = de_b_dp * dp_drhoEA;

    Real E_b, dE_b_de_b, dE_b_dvel_b;
    THM::E_from_e_vel(e_b, _vel, E_b, dE_b_de_b, dE_b_dvel_b);
    const Real dE_b_drhoA = dE_b_de_b * de_b_drhoA;
    const Real dE_b_drhouA = dE_b_de_b * de_b_drhouA;
    const Real dE_b_drhoEA = dE_b_de_b * de_b_drhoEA;

    const Real drhoEA_b_drhoA = rhoA_b * dE_b_drhoA;
    const Real drhoEA_b_drhouA = rhoA_b * dE_b_drhouA;
    const Real drhoEA_b_drhoEA = rhoA_b * dE_b_drhoEA;

    // compute ghost solution Jacobian

    J(THM3Eqn::EQ_ENERGY, THM3Eqn::EQ_MASS) = drhoEA_b_drhoA;
    J(THM3Eqn::EQ_ENERGY, THM3Eqn::EQ_MOMENTUM) = drhoEA_b_drhouA;
    J(THM3Eqn::EQ_ENERGY, THM3Eqn::EQ_ENERGY) = drhoEA_b_drhoEA;
  }
  else
  {
    J(THM3Eqn::EQ_MASS, THM3Eqn::EQ_MASS) = 1.;
    J(THM3Eqn::EQ_MOMENTUM, THM3Eqn::EQ_MASS) = _vel;
    J(THM3Eqn::EQ_ENERGY, THM3Eqn::EQ_ENERGY) = 1;
  }
  return J;
}
