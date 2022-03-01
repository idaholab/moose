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
 * Base class for function objects.  Functions override value to supply a
 * value at a point.
 */
class CompositeFunction : public Function, protected FunctionInterface
{
public:
  static InputParameters validParams();

  CompositeFunction(const InputParameters & parameters);

  using Function::value;
  virtual Real value(Real t, const Point & pt) const override;
  virtual ADReal value(const ADReal & t, const ADPoint & p) const override;

private:
  const Real _scale_factor;
  std::vector<const Function *> _f;
};
