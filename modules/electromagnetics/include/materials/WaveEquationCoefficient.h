//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADMaterial.h"

/**
 *  Material for use as coefficient $a k^2 mu_r epsilon_r$ (where a
 *  is a scalar coefficient) in standard-form Helmholtz wave equation
 *  applications with derivatives calculated using automatic differentiation.
 */
class WaveEquationCoefficient : public ADMaterial
{
public:
  static InputParameters validParams();

  WaveEquationCoefficient(const InputParameters & parameters);

  virtual void computeQpProperties() override;

protected:
  /// Real component of the relative electric permittivity
  const ADMaterialProperty<Real> & _eps_r_real;

  /// Imaginary component of the relative electric permittivity
  const ADMaterialProperty<Real> & _eps_r_imag;

  /// Real component of the relative magnetic permeability
  const ADMaterialProperty<Real> & _mu_r_real;

  /// Real component of the relative magnetic permeability
  const ADMaterialProperty<Real> & _mu_r_imag;

  /// Real component of the wave number
  const ADMaterialProperty<Real> & _k_real;

  /// Imaginary component of the wave number (also known as the attenuation constant)
  const ADMaterialProperty<Real> & _k_imag;

  /// Real-valued coefficient (defaults to 1)
  Real _coef;

  /// Material property name for the real component
  const MaterialPropertyName & _prop_name_real;

  /// Material property name for the imaginary component
  const MaterialPropertyName & _prop_name_imag;

  /// Material property for the real component
  ADMaterialProperty<Real> & _prop_real;

  /// Material property for the imaginary component
  ADMaterialProperty<Real> & _prop_imag;
};
