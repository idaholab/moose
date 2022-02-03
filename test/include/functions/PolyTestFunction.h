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

class PolyTestFunction : public Function
{
public:
  static InputParameters validParams();

  PolyTestFunction(const InputParameters & parameters);

  using Function::value;
  virtual Real value(Real t, const Point & p) const override;

  const std::vector<Real> _coeffs;
  const bool _deriv;
};
