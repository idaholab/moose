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

class ADMatWaveEquation : public ADVectorKernel
{
public:
  static InputParameters validParams();

  ADMatWaveEquation(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;
  // virtual Real computeQpJacobian() override;

private:
  /// Function coefficient
  // const Function & _func;

  /// Real component of the current source
  // const Function & _source_real;

  /// Imaginary component of the current source
  // const Function & _source_imag;

  const ADVectorVariableValue & _E_real;
  const ADVectorVariableValue & _E_imag;

  const ADMaterialProperty<Real> & _coef_real;
  const ADMaterialProperty<Real> & _coef_imag;

  /// Component of the field vector (real or imaginary)
  MooseEnum _component;
};
