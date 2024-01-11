//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Including the "ADKernel" base class here so we can extend it
#include "ADKernelGrad.h"

/**
 * Computes the residual contribution: K / mu * grad_test * grad_u.
 */
class DarcyPressure : public ADKernelGrad
{
public:
  static InputParameters validParams();

  DarcyPressure(const InputParameters & parameters);

protected:
  /// ADKernel objects must override precomputeQpResidual
  virtual ADRealVectorValue precomputeQpResidual() override;

  /// The material properties which hold the values for K and mu
  const ADMaterialProperty<Real> & _permeability;
  const ADMaterialProperty<Real> & _viscosity;
};
