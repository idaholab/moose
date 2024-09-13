//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADVectorKernel.h"

class ADConductiveCurrent : public ADVectorKernel
{
public:
  static InputParameters validParams();

  ADConductiveCurrent(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

private:
  const ADVectorVariableValue & _E_real;
  const ADVectorVariableValue & _E_imag;

  const ADMaterialProperty<Real> & _cond_real;
  const ADMaterialProperty<Real> & _cond_imag;

  const ADMaterialProperty<Real> & _omega_real;
  const ADMaterialProperty<Real> & _omega_imag;

  const ADMaterialProperty<Real> & _mu_real;
  const ADMaterialProperty<Real> & _mu_imag;

  /// Component of the field vector (real or imaginary)
  MooseEnum _component;
};
