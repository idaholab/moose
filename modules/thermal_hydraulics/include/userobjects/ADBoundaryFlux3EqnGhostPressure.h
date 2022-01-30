#pragma once

#include "ADBoundaryFlux3EqnGhostBase.h"

class SinglePhaseFluidProperties;

/**
 * Computes boundary flux from a specified pressure for the 1-D, 1-phase, variable-area Euler
 * equations
 */
class ADBoundaryFlux3EqnGhostPressure : public ADBoundaryFlux3EqnGhostBase
{
public:
  ADBoundaryFlux3EqnGhostPressure(const InputParameters & parameters);

protected:
  virtual std::vector<ADReal> getGhostCellSolution(const std::vector<ADReal> & U1) const override;

  /// Specified pressure
  const Real & _p;

  /// Fluid properties object
  const SinglePhaseFluidProperties & _fp;

public:
  static InputParameters validParams();
};
