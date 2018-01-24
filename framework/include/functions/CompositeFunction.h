//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPOSITEFUNCTION_H
#define COMPOSITEFUNCTION_H

#include "Function.h"
#include "FunctionInterface.h"

class CompositeFunction;

template <>
InputParameters validParams<CompositeFunction>();

/**
 * Base class for function objects.  Functions override value to supply a
 * value at a point.
 */
class CompositeFunction : public Function, protected FunctionInterface
{
public:
  CompositeFunction(const InputParameters & parameters);

  virtual Real value(Real t, const Point & pt) override;

private:
  const Real _scale_factor;
  std::vector<Function *> _f;
};

#endif // COMPOSITE_H
