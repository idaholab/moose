#pragma once

#include "ADBoundaryFlux3EqnGhostBase.h"

/**
 * Outflow boundary flux from a ghost cell for the 1-D, 1-phase, variable-area Euler equations
 */
class ADBoundaryFlux3EqnGhostFreeOutflow : public ADBoundaryFlux3EqnGhostBase
{
public:
  ADBoundaryFlux3EqnGhostFreeOutflow(const InputParameters & parameters);

protected:
  virtual std::vector<ADReal> getGhostCellSolution(const std::vector<ADReal> & U1) const override;

public:
  static InputParameters validParams();
};
