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

class CoarsendPiecewiseLinear;

template <>
InputParameters validParams<CoarsendPiecewiseLinear>();

/**
 * Perform a point reduction of the tabulated data upon initialization.
 */
class CoarsendPiecewiseLinear : public PiecewiseLinearBase
{
public:
  CoarsendPiecewiseLinear(const InputParameters & parameters);
};
