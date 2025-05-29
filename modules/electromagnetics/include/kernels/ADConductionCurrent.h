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
 *  the conduction formulation of the current.
 */
class ADConductionCurrent : public ADVectorKernel
{
public:
  static InputParameters validParams();

  ADConductionCurrent(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

private:
  /// Vector variable of the real component of the electric field
  const ADVectorVariableValue & _field_real;

  /// Vector variable of the imaginary component of the electric field
  const ADVectorVariableValue & _field_imag;

  /// Real component of the material conductivity (in S/m)
  const ADMaterialProperty<Real> & _cond_real;

  /// Imaginary component of the material conductivity (in S/m)
  const ADMaterialProperty<Real> & _cond_imag;

  /// Real component of the angular drive frequency (in rad/s)
  const ADMaterialProperty<Real> & _omega_real;

  /// Imaginary component of the angular drive frequency (in rad/s)
  const ADMaterialProperty<Real> & _omega_imag;

  /// Real component of the material permeability (in H/m)
  const ADMaterialProperty<Real> & _mu_real;

  /// Imaginary component of the material permeability (in H/m)
  const ADMaterialProperty<Real> & _mu_imag;

  /// Component of the field vector (real or imaginary)
  MooseEnum _component;
};
