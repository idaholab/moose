//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PiecewiseBase.h"

template <>
InputParameters
validParams<PiecewiseBase>()
{
  InputParameters params = validParams<Function>();

  MooseEnum axis("x=0 y=1 z=2 0=3 1=4 2=5");
  axis.deprecate("0", "x");
  axis.deprecate("1", "y");
  axis.deprecate("2", "z");
  params.addParam<MooseEnum>(
      "axis", axis, "The axis used (x, y, or z) if this is to be a function of position");
  return params;
}

PiecewiseBase::PiecewiseBase(const InputParameters & parameters)
  : Function(parameters), _has_axis(isParamValid("axis")), _data_set(false)
{
  if (_has_axis)
  {
    _axis = getParam<MooseEnum>("axis");
    if (_axis > 2)
      _axis -= 3;
  }
}

void
PiecewiseBase::initialSetup()
{
  if (!_data_set)
    mooseError("In ", _name, ": Data has not been set");
}

void
PiecewiseBase::setData(const std::vector<Real> & x, const std::vector<Real> & y)
{
  _data_set = true;
  // Size mismatch error
  if (x.size() != y.size())
    mooseError("In PiecewiseBase ", _name, ": Lengths of x and y data do not match.");

  try
  {
    _linear_interp = libmesh_make_unique<LinearInterpolation>(x, y);
  }
  catch (std::domain_error & e)
  {
    mooseError("In PiecewiseBase ", _name, ": ", e.what());
  }
}

Real
PiecewiseBase::functionSize() const
{
  return _linear_interp->getSampleSize();
}

Real
PiecewiseBase::domain(const int i) const
{
  return _linear_interp->domain(i);
}

Real
PiecewiseBase::range(const int i) const
{
  return _linear_interp->range(i);
}
