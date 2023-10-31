//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADBoundaryFlux3EqnGhostBase.h"
#include "ADFunctorInterface.h"

class SinglePhaseFluidProperties;

/**
 * Computes boundary flux from a functor pressure for the 1-D, 1-phase, variable-area Euler
 * equations
 */
class ADBoundaryFlux3EqnGhostFunctorPressure : public ADBoundaryFlux3EqnGhostBase,
                                               public ADFunctorInterface
{
public:
  ADBoundaryFlux3EqnGhostFunctorPressure(const InputParameters & parameters);

protected:
  virtual std::vector<ADReal> getGhostCellSolution(const std::vector<ADReal> & U1) const override;

  /// Specified pressure
  const Moose::Functor<ADReal> & _p;

  /// Fluid properties object
  const SinglePhaseFluidProperties & _fp;

public:
  static InputParameters validParams();
};
