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
template <typename BaseClass>
class PiecewiseBaseTempl : public BaseClass
{
public:
  static InputParameters validParams();

  PiecewiseBaseTempl(const InputParameters & parameters);

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

  using BaseClass::_name;
};

class PiecewiseBase : public PiecewiseBaseTempl<Function>
{
public:
  PiecewiseBase(const InputParameters & params) : PiecewiseBaseTempl<Function>(params) {}
  static InputParameters validParams() { return PiecewiseBaseTempl<Function>::validParams(); }
};

typedef PiecewiseBaseTempl<ADFunction> ADPiecewiseBase;
