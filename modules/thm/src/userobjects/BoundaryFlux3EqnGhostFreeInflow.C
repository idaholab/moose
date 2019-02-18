#include "BoundaryFlux3EqnGhostFreeInflow.h"
#include "THMIndices3Eqn.h"

registerMooseObject("THMApp", BoundaryFlux3EqnGhostFreeInflow);

template <>
InputParameters
validParams<BoundaryFlux3EqnGhostFreeInflow>()
{
  InputParameters params = validParams<BoundaryFlux3EqnGhostBase>();

  params.addClassDescription("Free inflow boundary conditions from a ghost cell for the 1-D, "
                             "1-phase, variable-area Euler equations");

  params.addRequiredParam<Real>("rho_infinity", "Far-stream density value");
  params.addRequiredParam<Real>("vel_infinity", "Far-stream velocity value");
  params.addRequiredParam<Real>("p_infinity", "Far-stream pressure value");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name of fluid properties user object");

  return params;
}

BoundaryFlux3EqnGhostFreeInflow::BoundaryFlux3EqnGhostFreeInflow(const InputParameters & parameters)
  : BoundaryFlux3EqnGhostBase(parameters),

    _rho_inf(getParam<Real>("rho_infinity")),
    _vel_inf(getParam<Real>("vel_infinity")),
    _p_inf(getParam<Real>("p_infinity")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
}

std::vector<Real>
BoundaryFlux3EqnGhostFreeInflow::getGhostCellSolution(const std::vector<Real> & U1) const
{
  const Real A = U1[THM3Eqn::CONS_VAR_AREA];

  const Real e_inf = _fp.e_from_p_rho(_p_inf, _rho_inf);
  const Real E_inf = e_inf + 0.5 * _vel_inf * _vel_inf;

  const Real rhoA = _rho_inf * A;
  const Real rhouA = rhoA * _vel_inf;
  const Real rhoEA = rhoA * E_inf;

  std::vector<Real> U_ghost(THM3Eqn::N_CONS_VAR);
  U_ghost[THM3Eqn::CONS_VAR_RHOA] = rhoA;
  U_ghost[THM3Eqn::CONS_VAR_RHOUA] = rhouA;
  U_ghost[THM3Eqn::CONS_VAR_RHOEA] = rhoEA;
  U_ghost[THM3Eqn::CONS_VAR_AREA] = A;

  return U_ghost;
}

DenseMatrix<Real>
BoundaryFlux3EqnGhostFreeInflow::getGhostCellSolutionJacobian(
    const std::vector<Real> & /*U1*/) const
{
  DenseMatrix<Real> J(THM3Eqn::N_EQ, THM3Eqn::N_EQ);
  return J;
}
