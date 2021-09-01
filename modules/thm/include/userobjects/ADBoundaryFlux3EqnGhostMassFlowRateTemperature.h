#pragma once

#include "ADBoundaryFlux3EqnGhostBase.h"

class SinglePhaseFluidProperties;

/**
 * Computes a boundary flux from a specified mass flow rate and temperature for
 * the 1-D, 1-phase, variable-area Euler equations using a ghost cell.
 */
class ADBoundaryFlux3EqnGhostMassFlowRateTemperature : public ADBoundaryFlux3EqnGhostBase
{
public:
  ADBoundaryFlux3EqnGhostMassFlowRateTemperature(const InputParameters & parameters);

protected:
  virtual std::vector<ADReal> getGhostCellSolution(const std::vector<ADReal> & U) const override;

  /// Specified mass flow rate
  const Real & _rhouA;

  /// Specified temperature
  const Real & _T;
  /// Reversible flag
  const bool & _reversible;

  /// Fluid properties object
  const SinglePhaseFluidProperties & _fp;

public:
  static InputParameters validParams();
};
