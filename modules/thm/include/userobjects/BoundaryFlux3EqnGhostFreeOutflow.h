#pragma once

#include "BoundaryFlux3EqnGhostBase.h"

class BoundaryFlux3EqnGhostFreeOutflow;

template <>
InputParameters validParams<BoundaryFlux3EqnGhostFreeOutflow>();

/**
 * Outflow boundary flux from a ghost cell for the 1-D, 1-phase, variable-area Euler equations
 */
class BoundaryFlux3EqnGhostFreeOutflow : public BoundaryFlux3EqnGhostBase
{
public:
  BoundaryFlux3EqnGhostFreeOutflow(const InputParameters & parameters);

protected:
  virtual std::vector<Real> getGhostCellSolution(const std::vector<Real> & U1) const override;
  virtual DenseMatrix<Real>
  getGhostCellSolutionJacobian(const std::vector<Real> & U1) const override;
};
