//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POLYTESTFUNCTION_H
#define POLYTESTFUNCTION_H

#include "Function.h"

class PolyTestFunction;

template <>
InputParameters validParams<PolyTestFunction>();

class PolyTestFunction : public Function
{
public:
  PolyTestFunction(const InputParameters & parameters);

  virtual Real value(Real t, const Point & p);

  const std::vector<Real> _coeffs;
  const bool _deriv;
};

#endif // POLYTESTFUNCTION_H
