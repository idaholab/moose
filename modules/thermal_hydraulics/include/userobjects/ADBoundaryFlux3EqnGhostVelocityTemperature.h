//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADBoundaryFlux3EqnGhostBase.h"

class SinglePhaseFluidProperties;
class Function;

/**
 * Computes a boundary flux from a specified velocity and temperature for
 * the 1-D, 1-phase, variable-area Euler equations using a ghost cell.
 */
class ADBoundaryFlux3EqnGhostVelocityTemperature : public ADBoundaryFlux3EqnGhostBase
{
public:
  ADBoundaryFlux3EqnGhostVelocityTemperature(const InputParameters & parameters);

protected:
  virtual std::vector<ADReal> getGhostCellSolution(const std::vector<ADReal> & U,
                                                   const Point & point) const override;

  /// Function specifying the velocity
  const Function & _vel_fn;

  /// Function specifying the temperature
  const Function & _T_fn;
  /// Reversible flag
  const bool & _reversible;

  /// Fluid properties object
  const SinglePhaseFluidProperties & _fp;

public:
  static InputParameters validParams();
};
