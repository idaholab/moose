//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Including the "ADKernel" Kernel here so we can extend it
#include "Kernel.h"

/**
 * Computes the residual contribution: K / mu * grad_u * grad_phi.
 */
class DarcyPressure : public Kernel
{
public:
  static InputParameters validParams();

  DarcyPressure(const InputParameters & parameters);

protected:
  /// Kernel objects must override
  virtual Real computeQpResidual() override;

  /// Kernel objects optionally override
  virtual Real computeQpJacobian() override;

  /// References to be set from input file
  const Real & _permeability;
  const Real & _viscosity;
};
