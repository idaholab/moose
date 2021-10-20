//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVTimeKernel.h"

class PCNSFVEnergyTimeDerivative : public FVTimeKernel
{
public:
  static InputParameters validParams();
  PCNSFVEnergyTimeDerivative(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  /// the density
  const ADMaterialProperty<Real> & _rho;
  /// the density time derivative
  const ADMaterialProperty<Real> * _rho_dot;
  /// the heat conductivity
  const ADMaterialProperty<Real> & _cp;
  /// the heat conductivity time derivative
  const ADMaterialProperty<Real> * _cp_dot;
  /// the porosity
  const VariableValue & _eps;
  /// whether this kernel is being used for a solid or a fluid temperature
  const bool _is_solid;
  /// scales the value of the kernel, used for faster steady state during pseudo transient
  const Real _scaling;
  /// whether a zero scaling factor has been specifed
  const bool _zero_scaling;
};
