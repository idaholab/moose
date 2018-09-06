#include "BoundaryFlux3EqnGhostWall.h"
#include "RELAP7Indices3Eqn.h"

registerMooseObject("RELAP7App", BoundaryFlux3EqnGhostWall);

template <>
InputParameters
validParams<BoundaryFlux3EqnGhostWall>()
{
  InputParameters params = validParams<BoundaryFlux3EqnGhostBase>();

  params.addClassDescription(
      "Wall boundary conditions for the 1-D, 1-phase, variable-area Euler equations");

  return params;
}

BoundaryFlux3EqnGhostWall::BoundaryFlux3EqnGhostWall(const InputParameters & parameters)
  : BoundaryFlux3EqnGhostBase(parameters)
{
}

std::vector<Real>
BoundaryFlux3EqnGhostWall::getGhostCellSolution(const std::vector<Real> & U1) const
{
  std::vector<Real> U_ghost(RELAP73Eqn::N_CONS_VAR);
  U_ghost[RELAP73Eqn::CONS_VAR_RHOA] = U1[RELAP73Eqn::CONS_VAR_RHOA];
  U_ghost[RELAP73Eqn::CONS_VAR_RHOUA] = -U1[RELAP73Eqn::CONS_VAR_RHOUA];
  U_ghost[RELAP73Eqn::CONS_VAR_RHOEA] = U1[RELAP73Eqn::CONS_VAR_RHOEA];
  U_ghost[RELAP73Eqn::CONS_VAR_AREA] = U1[RELAP73Eqn::CONS_VAR_AREA];

  return U_ghost;
}

DenseMatrix<Real>
BoundaryFlux3EqnGhostWall::getGhostCellSolutionJacobian(const std::vector<Real> & /*U1*/) const
{
  DenseMatrix<Real> J(RELAP73Eqn::N_EQ, RELAP73Eqn::N_EQ);
  J(RELAP73Eqn::EQ_MASS, RELAP73Eqn::EQ_MASS) = 1.0;
  J(RELAP73Eqn::EQ_MOMENTUM, RELAP73Eqn::EQ_MOMENTUM) = -1.0;
  J(RELAP73Eqn::EQ_ENERGY, RELAP73Eqn::EQ_ENERGY) = 1.0;

  return J;
}
