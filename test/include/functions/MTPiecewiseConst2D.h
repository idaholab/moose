//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MTPIECEWISECONST2D_H
#define MTPIECEWISECONST2D_H

#include "Function.h"

class MTPiecewiseConst2D;

template <>
InputParameters validParams<MTPiecewiseConst2D>();

class MTPiecewiseConst2D : public Function
{
public:
  MTPiecewiseConst2D(const InputParameters & parameters);

  virtual Real value(Real t, const Point & p);
};

#endif // MTPIECEWISECONST2D_H
