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
 * Kernel which implements the convective term in the transient heat
 * conduction equation, and provides coupling with the Darcy pressure
 * equation.
 */
class DarcyAdvection : public ADKernelValue
{
public:
  static InputParameters validParams();

  DarcyAdvection(const InputParameters & parameters);

protected:
  /// ADKernelValue objects must override precomputeQpResidual
  virtual ADReal precomputeQpResidual() override;

  /// The gradient of pressure
  const ADVariableGradient & _pressure_grad;

  /// These references will be set by the initialization list so that
  /// values can be pulled from the Material system.
  const ADMaterialProperty<Real> & _permeability;
  const ADMaterialProperty<Real> & _porosity;
  const ADMaterialProperty<Real> & _viscosity;
  const ADMaterialProperty<Real> & _density;
  const ADMaterialProperty<Real> & _specific_heat;
};
