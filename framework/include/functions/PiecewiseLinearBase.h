//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PiecewiseTabularBase.h"

/**
 * Base class for functions which provides a piecewise continuous linear
 * interpolation of an (x,y) point data set.
 */
template <typename BaseClass>
class PiecewiseLinearBaseTempl : public BaseClass
{
public:
  static InputParameters validParams();

  PiecewiseLinearBaseTempl(const InputParameters & parameters);

  virtual void initialSetup() override;

  using BaseClass::value;
  virtual Real value(Real t, const Point & p) const override;
  virtual ADReal value(const ADReal & t, const ADPoint & p) const override;

  virtual Real timeDerivative(Real t, const Point &) const override;
  virtual RealGradient gradient(Real, const Point & p) const override;
  virtual Real integral() const override;
  virtual Real average() const override;
  virtual void setData(const std::vector<Real> & x, const std::vector<Real> & y) override;

protected:
  /**
   * Builds the linear interpolation object from the x/y data
   *
   * @param[in] extrap  Extrapolate when sample point is outside of bounds?
   */
  void buildInterpolation(const bool extrap = false);

  /// helper object to perform the linear interpolation of the function data
  std::unique_ptr<LinearInterpolation> _linear_interp;

  using BaseClass::_axis;
  using BaseClass::_has_axis;
  using BaseClass::_name;
  using BaseClass::_raw_x;
  using BaseClass::_raw_y;
  using BaseClass::_scale_factor;
};

class PiecewiseLinearBase : public PiecewiseLinearBaseTempl<PiecewiseTabularBase>
{
public:
  PiecewiseLinearBase(const InputParameters & params)
    : PiecewiseLinearBaseTempl<PiecewiseTabularBase>(params)
  {
  }
  static InputParameters validParams()
  {
    return PiecewiseLinearBaseTempl<PiecewiseTabularBase>::validParams();
  }
};

typedef PiecewiseLinearBaseTempl<ADPiecewiseTabularBase> ADPiecewiseLinearBase;
