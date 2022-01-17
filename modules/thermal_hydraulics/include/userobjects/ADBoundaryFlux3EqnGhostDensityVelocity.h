#pragma once

#include "ADBoundaryFlux3EqnGhostBase.h"

class SinglePhaseFluidProperties;

/**
 * Computes boundary flux from densities and velocities for the 3-equation model
 * using a ghost cell approach.
 */
class ADBoundaryFlux3EqnGhostDensityVelocity : public ADBoundaryFlux3EqnGhostBase
{
public:
  ADBoundaryFlux3EqnGhostDensityVelocity(const InputParameters & parameters);

protected:
  virtual std::vector<ADReal>
  getGhostCellSolution(const std::vector<ADReal> & U_interior) const override;

  /// Specified density
  const Real & _rho;
  /// Specified velocity
  const Real & _vel;
  /// Reversibility
  const bool & _reversible;
  /// Fluid properties object
  const SinglePhaseFluidProperties & _fp;

public:
  static InputParameters validParams();
};
