//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
class ADMatWaveReaction : public ADVectorKernel
{
public:
  static InputParameters validParams();

  ADMatWaveReaction(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

private:
  /// Vector variable of the real component of the electric field
  const ADVectorVariableValue & _field_real;

  /// Vector variable of the imaginary component of the electric field
  const ADVectorVariableValue & _field_imag;

  /// The real component of the coefficient for the Helmholtz wave equation
  const ADMaterialProperty<Real> & _coef_real;

  /// The imaginary component of the coefficient for the Helmholtz wave equation
  const ADMaterialProperty<Real> & _coef_imag;

  /// Component of the field vector (real or imaginary)
  MooseEnum _component;
};
