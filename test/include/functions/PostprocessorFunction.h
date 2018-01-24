//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POSTPROCESSORFUNCTION_H
#define POSTPROCESSORFUNCTION_H

#include "Function.h"

class PostprocessorFunction;

template <>
InputParameters validParams<PostprocessorFunction>();

class PostprocessorFunction : public Function
{
public:
  PostprocessorFunction(const InputParameters & parameters);

  virtual Real value(Real t, const Point & p);

protected:
  const PostprocessorValue & _pp;
};

#endif // POSTPROCESSORFUNCTION_H
