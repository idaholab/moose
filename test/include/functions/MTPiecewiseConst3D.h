//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MTPIECEWISECONST3D_H
#define MTPIECEWISECONST3D_H

#include "Function.h"

class MTPiecewiseConst3D;

template <>
InputParameters validParams<MTPiecewiseConst3D>();

class MTPiecewiseConst3D : public Function
{
public:
  MTPiecewiseConst3D(const InputParameters & parameters);

  virtual Real value(Real t, const Point & p);
};

#endif // MTPIECEWISECONST3D_H
