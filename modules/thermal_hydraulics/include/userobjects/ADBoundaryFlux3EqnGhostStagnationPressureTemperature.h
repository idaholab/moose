#pragma once

#include "ADBoundaryFlux3EqnGhostBase.h"

class SinglePhaseFluidProperties;

/**
 * Computes boundary flux from a specified stagnation pressure and temperature
 * for the 1-D, 1-phase, variable-area Euler equations
 */
class ADBoundaryFlux3EqnGhostStagnationPressureTemperature : public ADBoundaryFlux3EqnGhostBase
{
public:
  ADBoundaryFlux3EqnGhostStagnationPressureTemperature(const InputParameters & parameters);

protected:
  virtual std::vector<ADReal> getGhostCellSolution(const std::vector<ADReal> & U1) const override;

  /// Specified stagnation pressure
  const Real & _p0;

  /// Specified stagnation temperature
  const Real & _T0;
  /// Reversibility
  const bool & _reversible;

  /// Fluid properties object
  const SinglePhaseFluidProperties & _fp;

public:
  static InputParameters validParams();
};
