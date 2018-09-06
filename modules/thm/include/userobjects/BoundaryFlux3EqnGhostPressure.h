#ifndef BOUNDARYFLUX3EQNGHOSTPRESSURE_H
#define BOUNDARYFLUX3EQNGHOSTPRESSURE_H

#include "BoundaryFlux3EqnGhostBase.h"

class BoundaryFlux3EqnGhostPressure;
class SinglePhaseFluidProperties;

template <>
InputParameters validParams<BoundaryFlux3EqnGhostPressure>();

/**
 * Computes boundary flux from a specified pressure for the 1-D, 1-phase, variable-area Euler
 * equations
 */
class BoundaryFlux3EqnGhostPressure : public BoundaryFlux3EqnGhostBase
{
public:
  BoundaryFlux3EqnGhostPressure(const InputParameters & parameters);

protected:
  virtual std::vector<Real> getGhostCellSolution(const std::vector<Real> & U1) const override;
  virtual DenseMatrix<Real>
  getGhostCellSolutionJacobian(const std::vector<Real> & U1) const override;

  /// Specified pressure
  const Real & _p;

  /// Fluid properties object
  const SinglePhaseFluidProperties & _fp;
};

#endif
