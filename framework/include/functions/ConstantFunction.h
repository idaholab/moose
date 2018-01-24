//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CONSTANTFUNCTION_H
#define CONSTANTFUNCTION_H

#include "Function.h"

class ConstantFunction;

template <>
InputParameters validParams<ConstantFunction>();

/**
 * Class that represents constant function
 */
class ConstantFunction : public Function
{
public:
  ConstantFunction(const InputParameters & parameters);

  virtual Real value(Real t, const Point & p) override;

protected:
  const Real & _value;
};

#endif
