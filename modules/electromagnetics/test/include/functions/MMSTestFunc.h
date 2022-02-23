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
  Real _length;

  const Real _g_0_real;
  const Real _g_0_imag;

  const Real _g_l_real;
  const Real _g_l_imag;

  const MooseEnum _component;
};
