//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef EXAMPLEFUNCTION_H
#define EXAMPLEFUNCTION_H

#include "Function.h"

class ExampleFunction;

template <>
InputParameters validParams<ExampleFunction>();

class ExampleFunction : public Function
{
public:
  ExampleFunction(const InputParameters & parameters);

  virtual Real value(Real t, const Point & p) override;

protected:
  Real _alpha;
};

#endif // EXAMPLEFUNCTION_H
