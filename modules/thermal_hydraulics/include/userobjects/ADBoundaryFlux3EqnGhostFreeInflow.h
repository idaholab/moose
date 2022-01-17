#pragma once

#include "ADBoundaryFlux3EqnGhostBase.h"
#include "SinglePhaseFluidProperties.h"

/**
 * Free inflow boundary conditions from a ghost cell for the 1-D, 1-phase, variable-area Euler
 * equations
 */
class ADBoundaryFlux3EqnGhostFreeInflow : public ADBoundaryFlux3EqnGhostBase
{
public:
  ADBoundaryFlux3EqnGhostFreeInflow(const InputParameters & parameters);

protected:
  virtual std::vector<ADReal> getGhostCellSolution(const std::vector<ADReal> & U1) const override;

  /// farstream density
  const Real _rho_inf;
  /// farstream velocity
  const Real _vel_inf;
  /// farstream pressure
  const Real _p_inf;

  /// fluid properties object
  const SinglePhaseFluidProperties & _fp;

public:
  static InputParameters validParams();
};
