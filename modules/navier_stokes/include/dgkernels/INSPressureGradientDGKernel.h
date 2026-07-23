//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADDGKernel.h"

/**
 * Adds the centered interior-face pressure contribution for a discontinuous momentum equation
 * whose pressure gradient is integrated by parts.
 */
class INSPressureGradientDGKernel : public ADDGKernel
{
public:
  static InputParameters validParams();

  INSPressureGradientDGKernel(const InputParameters & parameters);

  virtual void initialSetup() override;

protected:
  virtual ADReal computeQpResidual(Moose::DGResidualType type) override;

  /// Pressure on the current element side.
  const ADVariableValue & _pressure;

  /// Pressure on the neighboring element side.
  const ADVariableValue & _pressure_neighbor;

  /// Momentum component to which this pressure contribution is added.
  const unsigned _component;
};
