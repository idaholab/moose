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

/**
 * Function base which provides a piecewise approximation to a specified (x,y) point data set.
 * Derived classes can either directly implement the x/y data, or provide input parameter mechanisms
 * for such data formulation.
 */
class PiecewiseBase : public Function
{
public:
  static InputParameters validParams();

  PiecewiseBase(const InputParameters & parameters);

  virtual Real functionSize() const;
  virtual Real domain(const int i) const;
  virtual Real range(const int i) const;

  /**
   * Provides a means for explicitly setting the x and y data. This must
   * be called in the constructor of inherited classes.
   */
  virtual void setData(const std::vector<Real> & x, const std::vector<Real> & y);

protected:
  ///@{ raw function data as read
  std::vector<Real> _raw_x;
  std::vector<Real> _raw_y;
  ///@}

  using Function::_name;
};
