//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PiecewiseBase.h"
#include "DelimitedFileReader.h"

InputParameters
PiecewiseBase::validParams()
{
  return Function::validParams();
}

PiecewiseBase::PiecewiseBase(const InputParameters & parameters) : Function(parameters) {}

Real
PiecewiseBase::functionSize() const
{
  return _raw_x.size();
}

Real
PiecewiseBase::domain(const int i) const
{
  return _raw_x[i];
}

Real
PiecewiseBase::range(const int i) const
{
  return _raw_y[i];
}

void
PiecewiseBase::setData(const std::vector<Real> & x, const std::vector<Real> & y)
{
  _raw_x = x;
  _raw_y = y;
  if (_raw_x.size() != _raw_y.size())
    mooseError("In PiecewiseBase ", _name, ": Lengths of x and y data do not match.");
}
