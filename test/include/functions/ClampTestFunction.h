//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CLAMPTESTFUNCTION_H
#define CLAMPTESTFUNCTION_H

#include "Function.h"

class ClampTestFunction;

template <>
InputParameters validParams<ClampTestFunction>();

class ClampTestFunction : public Function
{
public:
  ClampTestFunction(const InputParameters & parameters);

  virtual Real value(Real t, const Point & p);
};

#endif // CLAMPTESTFUNCTION_H
