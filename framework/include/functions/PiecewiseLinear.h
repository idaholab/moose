//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PiecewiseLinearBase.h"

/**
 * Function which provides a piecewise continuous linear interpolation
 * of a provided (x,y) point data set.
 */
template <typename BaseClass>
class PiecewiseLinearTempl : public BaseClass
{
public:
  static InputParameters validParams();

  PiecewiseLinearTempl(const InputParameters & parameters);
};

class PiecewiseLinear : public PiecewiseLinearTempl<PiecewiseLinearBase>
{
public:
  PiecewiseLinear(const InputParameters & params)
    : PiecewiseLinearTempl<PiecewiseLinearBase>(params)
  {
  }
  static InputParameters validParams()
  {
    return PiecewiseLinearTempl<PiecewiseLinearBase>::validParams();
  }
};

typedef PiecewiseLinearTempl<ADPiecewiseLinearBase> ADPiecewiseLinear;
