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

template <typename BaseClass>
InputParameters
PiecewiseBaseTempl<BaseClass>::validParams()
{
  return BaseClass::validParams();
}

template <typename BaseClass>
PiecewiseBaseTempl<BaseClass>::PiecewiseBaseTempl(const InputParameters & parameters)
  : BaseClass(parameters)
{
}

template <typename BaseClass>
Real
PiecewiseBaseTempl<BaseClass>::functionSize() const
{
  return _raw_x.size();
}

template <typename BaseClass>
Real
PiecewiseBaseTempl<BaseClass>::domain(const int i) const
{
  return _raw_x[i];
}

template <typename BaseClass>
Real
PiecewiseBaseTempl<BaseClass>::range(const int i) const
{
  return _raw_y[i];
}

template <typename BaseClass>
void
PiecewiseBaseTempl<BaseClass>::setData(const std::vector<Real> & x, const std::vector<Real> & y)
{
  _raw_x = x;
  _raw_y = y;
  if (_raw_x.size() != _raw_y.size())
    mooseError("In PiecewiseBase ", _name, ": Lengths of x and y data do not match.");
}

template class PiecewiseBaseTempl<Function>;
template class PiecewiseBaseTempl<ADFunction>;
