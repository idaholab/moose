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

/**
 *  Calculates the current source term in the Helmholtz wave equation using 
 *  the dielectric formulation of the current.
 */
class ADMatWaveEquation : public ADVectorKernel
{
public:
  static InputParameters validParams();

  ADMatWaveEquation(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

private:

  /// Vector variable of the real component of the E-field
  const ADVectorVariableValue & _E_real;

  /// Vector variable of the imaginary component of the E-field
  const ADVectorVariableValue & _E_imag;

  /// The real component of the coefficient for the Helmholtz wave equation
  const ADMaterialProperty<Real> & _coef_real;
  
  /// The imaginary component of the coefficient for the Helmholtz wave equation
  const ADMaterialProperty<Real> & _coef_imag;

  /// Component of the field vector (real or imaginary)
  MooseEnum _component;
};
