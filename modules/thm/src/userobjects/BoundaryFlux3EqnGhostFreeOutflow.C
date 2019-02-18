#include "BoundaryFlux3EqnGhostFreeOutflow.h"
#include "THMIndices3Eqn.h"

registerMooseObject("THMApp", BoundaryFlux3EqnGhostFreeOutflow);

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
  DenseMatrix<Real> J(THM3Eqn::N_EQ, THM3Eqn::N_EQ);
  J(THM3Eqn::EQ_MASS, THM3Eqn::EQ_MASS) = 1;
  J(THM3Eqn::EQ_MOMENTUM, THM3Eqn::EQ_MOMENTUM) = 1;
  J(THM3Eqn::EQ_ENERGY, THM3Eqn::EQ_ENERGY) = 1;

  return J;
}
