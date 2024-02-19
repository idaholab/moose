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
 *  Function for field coefficient in slab reflection benchmark case.
 */
class JinSlabCoeffFunc : public Function
{
public:
  static InputParameters validParams();

  JinSlabCoeffFunc(const InputParameters & parameters);

  using Function::value;
  virtual Real value(Real t, const Point & p) const override;

protected:
  /// Wavenumber of incoming wave (1/m)
  const Real _k;

  /// Wave incidence angle, in degrees
  const Real _theta;

  /// Length of slab domain (m)
  const Real _length;

  /// Real-valued function coefficient
  const Real _coef;

  /// Enum signifying the component of the function being calculated
  const MooseEnum _component;
};
