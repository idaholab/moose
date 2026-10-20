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
 * Computes boundary flux from a specified stagnation pressure and temperature
 * for the 1-D, 1-phase, variable-area Euler equations
 */
class ADBoundaryFlux3EqnGhostStagnationPressureTemperature : public ADBoundaryFlux3EqnGhostBase
{
public:
  ADBoundaryFlux3EqnGhostStagnationPressureTemperature(const InputParameters & parameters);

protected:
  virtual std::vector<ADReal> getGhostCellSolution(const std::vector<ADReal> & U1,
                                                   const Point & point) const override;

  /// Function specifying the stagnation pressure
  const Function & _p0_fn;

  /// Function specifying the stagnation temperature
  const Function & _T0_fn;
  /// Reversibility
  const bool & _reversible;

  /// Fluid properties object
  const SinglePhaseFluidProperties & _fp;

public:
  static InputParameters validParams();
};
