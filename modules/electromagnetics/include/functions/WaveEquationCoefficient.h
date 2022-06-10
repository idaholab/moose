//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Function.h"
#include "FunctionInterface.h"

/**
 *  Function for use as coefficient in standard-form Helmholtz wave equation applications.
 */
class WaveEquationCoefficient : public Function, public FunctionInterface
{
public:
  static InputParameters validParams();

  WaveEquationCoefficient(const InputParameters & parameters);

  using Function::value;
  virtual Real value(Real t, const Point & p) const override;

protected:
  /// Real component of the relative electric permittivity
  const Function & _eps_r_real;

  /// Imaginary component of the relative electric permittivity
  const Function & _eps_r_imag;

  /// Real component of the relative magnetic permeability
  const Function & _mu_r_real;

  /// Real component of the relative magnetic permeability
  const Function & _mu_r_imag;

  /// Real component of the wave number
  const Function & _k_real;

  /// Imaginary component of the wave number (also known as the attenuation constant)
  const Function & _k_imag;

  /// Real-valued function coefficient (defaults to 1)
  Real _coef;

  /// Signifies whether function output should be the real or imaginary component
  const MooseEnum _component;
};
