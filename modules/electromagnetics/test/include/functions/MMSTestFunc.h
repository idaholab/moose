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

/**
 *  Function of RHS for manufactured solution in scalar_complex_helmholtz test
 */
class MMSTestFunc : public Function
{
public:
  static InputParameters validParams();
  MMSTestFunc(const InputParameters & parameters);

  using Function::value;
  virtual Real value(Real t, const Point & p) const override;

protected:
  /// Length of 1D test domain, where 0 < x < L
  const Real _length;

  /// Real component of DirichletBC, where x = 0
  const Real _g_0_real;

  /// Imaginary component of DirichletBC, where x = 0
  const Real _g_0_imag;

  /// Real component of DirichletBC, where x = L
  const Real _g_l_real;

  /// Imaginary component of DirichletBC, where x = L
  const Real _g_l_imag;

  /// Enum signifying the component of the function being calculated
  const MooseEnum _component;
};
