#pragma once

#include "BoundaryFlux3EqnGhostBase.h"

class BoundaryFlux3EqnGhostStagnationPressureTemperature;
class SinglePhaseFluidProperties;

template <>
InputParameters validParams<BoundaryFlux3EqnGhostStagnationPressureTemperature>();

/**
 * Computes boundary flux from a specified stagnation pressure and temperature
 * for the 1-D, 1-phase, variable-area Euler equations
 */
class BoundaryFlux3EqnGhostStagnationPressureTemperature : public BoundaryFlux3EqnGhostBase
{
public:
  BoundaryFlux3EqnGhostStagnationPressureTemperature(const InputParameters & parameters);

protected:
  virtual std::vector<Real> getGhostCellSolution(const std::vector<Real> & U1) const override;
  virtual DenseMatrix<Real>
  getGhostCellSolutionJacobian(const std::vector<Real> & U1) const override;

  /// Specified stagnation pressure
  const Real & _p0;

  /// Specified stagnation temperature
  const Real & _T0;

  /// Fluid properties object
  const SinglePhaseFluidProperties & _fp;
};
