#pragma once

#include "ADBoundaryFlux3EqnGhostBase.h"

/**
 * Computes flux for wall boundary conditions for the 1-D, 1-phase, variable-area Euler equations
 */
class ADBoundaryFlux3EqnGhostWall : public ADBoundaryFlux3EqnGhostBase
{
public:
  ADBoundaryFlux3EqnGhostWall(const InputParameters & parameters);

protected:
  virtual std::vector<ADReal> getGhostCellSolution(const std::vector<ADReal> & U1) const override;

public:
  static InputParameters validParams();
};
