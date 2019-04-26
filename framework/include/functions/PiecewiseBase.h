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
#include "LinearInterpolation.h"

class PiecewiseBase;

template <>
InputParameters validParams<PiecewiseBase>();

/**
 * Function base which provides a piecewise approximation to a provided
 * (x,y) point data set.  Derived classes which control the order
 * (constant, linear) of the approximation and how the (x,y) data set
 * is generated. should be used directly,
 */
class PiecewiseBase : public Function
{
public:
  PiecewiseBase(const InputParameters & parameters);

  virtual void initialSetup();
  virtual Real functionSize();
  virtual Real domain(const int i);
  virtual Real range(const int i);

  /**
   * Provides a means for explicitly setting the x and y data. This must
   * be called in the constructor of inhereted classes.
   */
  void setData(const std::vector<Real> & x, const std::vector<Real> & y);

protected:
  std::unique_ptr<LinearInterpolation> _linear_interp;
  int _axis;
  const bool _has_axis;
  bool _data_set;
};

