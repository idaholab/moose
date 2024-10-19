//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernelValue.h"

/**
 * Advection Kernel for the phasefield equation.
 *
 * u.grad(phi),
 * where u is the interface velocity and phi is the order parameter
 */
class PhaseFieldAdvection : public ADKernelValue
{
public:
  static InputParameters validParams();

  PhaseFieldAdvection(const InputParameters & parameters);

protected:
  virtual ADReal precomputeQpResidual() override;

  /// Velocity vector variable
  const ADVectorVariableValue & _velocity;
};
