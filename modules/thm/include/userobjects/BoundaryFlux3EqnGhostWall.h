#ifndef BOUNDARYFLUX3EQNGHOSTWALL_H
#define BOUNDARYFLUX3EQNGHOSTWALL_H

#include "BoundaryFlux3EqnGhostBase.h"

class BoundaryFlux3EqnGhostWall;

template <>
InputParameters validParams<BoundaryFlux3EqnGhostWall>();

/**
 * Computes flux for wall boundary conditions for the 1-D, 1-phase, variable-area Euler equations
 */
class BoundaryFlux3EqnGhostWall : public BoundaryFlux3EqnGhostBase
{
public:
  BoundaryFlux3EqnGhostWall(const InputParameters & parameters);

protected:
  virtual std::vector<Real> getGhostCellSolution(const std::vector<Real> & U1) const override;
  virtual DenseMatrix<Real>
  getGhostCellSolutionJacobian(const std::vector<Real> & U1) const override;
};

#endif
