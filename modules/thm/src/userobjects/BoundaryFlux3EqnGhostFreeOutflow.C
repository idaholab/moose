#include "BoundaryFlux3EqnGhostFreeOutflow.h"
#include "RELAP7Indices3Eqn.h"

registerMooseObject("RELAP7App", BoundaryFlux3EqnGhostFreeOutflow);

template <>
InputParameters
validParams<BoundaryFlux3EqnGhostFreeOutflow>()
{
  InputParameters params = validParams<BoundaryFlux3EqnGhostBase>();

  params.addClassDescription("Outflow boundary flux from a ghost cell for the 1-D, 1-phase, "
                             "variable-area Euler equations");

  return params;
}

BoundaryFlux3EqnGhostFreeOutflow::BoundaryFlux3EqnGhostFreeOutflow(
    const InputParameters & parameters)
  : BoundaryFlux3EqnGhostBase(parameters)
{
}

std::vector<Real>
BoundaryFlux3EqnGhostFreeOutflow::getGhostCellSolution(const std::vector<Real> & U1) const
{
  return U1;
}

DenseMatrix<Real>
BoundaryFlux3EqnGhostFreeOutflow::getGhostCellSolutionJacobian(
    const std::vector<Real> & /*U1*/) const
{
  DenseMatrix<Real> J(RELAP73Eqn::N_EQ, RELAP73Eqn::N_EQ);
  J(RELAP73Eqn::EQ_MASS, RELAP73Eqn::EQ_MASS) = 1;
  J(RELAP73Eqn::EQ_MOMENTUM, RELAP73Eqn::EQ_MOMENTUM) = 1;
  J(RELAP73Eqn::EQ_ENERGY, RELAP73Eqn::EQ_ENERGY) = 1;

  return J;
}
