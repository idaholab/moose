#pragma once

#include "ADBoundaryFlux3EqnGhostBase.h"

class SinglePhaseFluidProperties;

/**
 * Computes a boundary flux from a specified velocity and temperature for
 * the 1-D, 1-phase, variable-area Euler equations using a ghost cell.
 */
class ADBoundaryFlux3EqnGhostVelocityTemperature : public ADBoundaryFlux3EqnGhostBase
{
public:
  ADBoundaryFlux3EqnGhostVelocityTemperature(const InputParameters & parameters);

protected:
  virtual std::vector<ADReal> getGhostCellSolution(const std::vector<ADReal> & U) const override;

  /// Specified velocity
  const Real & _vel;

  /// Specified temperature
  const Real & _T;
  /// Reversible flag
  const bool & _reversible;

  /// Fluid properties object
  const SinglePhaseFluidProperties & _fp;

public:
  static InputParameters validParams();
};
