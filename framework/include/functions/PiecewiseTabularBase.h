//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PiecewiseBase.h"
#include "LinearInterpolation.h"

/**
 * Function base which provides a piecewise approximation to a provided (x,y) point data set via
 * input parameter specifications. Derived classes, which control the order (constant, linear) of
 * the approximation and how the (x,y) data set is generated, should be used directly.
 */
class PiecewiseTabularBase : public PiecewiseBase
{
public:
  static InputParameters validParams();

  PiecewiseTabularBase(const InputParameters & parameters);

protected:
  /// function value scale factor
  const Real & _scale_factor;

  ///@{ if _has_axis is true point component to use as function argument, otherwise use t
  int _axis;
  const bool _has_axis;
  ///@}

private:
  /// Reads data from supplied CSV file.
  void buildFromFile();

  /// Builds data from 'x' and 'y' parameters.
  void buildFromXandY();

  /// Builds data from 'xy_data' parameter.
  void buildFromXY();
};
