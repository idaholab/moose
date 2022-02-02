//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SplineInterpolation.h"

// MOOSE includes
#include "MooseError.h"

// C++ includes
#include <fstream>

int SplineInterpolation::_file_number = 0;

SplineInterpolation::SplineInterpolation() {}

SplineInterpolation::SplineInterpolation(const std::vector<Real> & x,
                                         const std::vector<Real> & y,
                                         Real yp1 /* = _deriv_bound*/,
                                         Real ypn /* = _deriv_bound*/)
  : SplineInterpolationBase(), _x(x), _y(y), _yp1(yp1), _ypn(ypn)
{
  errorCheck();
  solve();
}

void
SplineInterpolation::setData(const std::vector<Real> & x,
                             const std::vector<Real> & y,
                             Real yp1 /* = _deriv_bound*/,
                             Real ypn /* = _deriv_bound*/)
{
  _x = x;
  _y = y;
  _yp1 = yp1;
  _ypn = ypn;
  errorCheck();
  solve();
}

void
SplineInterpolation::errorCheck()
{
  if (_x.size() != _y.size())
    mooseError("SplineInterpolation: vectors are not the same length");

  bool error = false;
  for (unsigned i = 0; !error && i + 1 < _x.size(); ++i)
    if (_x[i] >= _x[i + 1])
      error = true;

  if (error)
    mooseError("x-values are not strictly increasing");
}

void
SplineInterpolation::solve()
{
  spline(_x, _y, _y2, _yp1, _ypn);
}

Real
SplineInterpolation::sample(Real x) const
{
  return SplineInterpolationBase::sample(_x, _y, _y2, x);
}

ADReal
SplineInterpolation::sample(const ADReal & x) const
{
  return SplineInterpolationBase::sample(_x, _y, _y2, x);
}

Real
SplineInterpolation::sampleDerivative(Real x) const
{
  return SplineInterpolationBase::sampleDerivative(_x, _y, _y2, x);
}

Real
SplineInterpolation::sample2ndDerivative(Real x) const
{
  return SplineInterpolationBase::sample2ndDerivative(_x, _y, _y2, x);
}

Real
SplineInterpolation::domain(int i) const
{
  return _x[i];
}

Real
SplineInterpolation::range(int i) const
{
  return _y[i];
}

unsigned int
SplineInterpolation::getSampleSize()
{
  return _x.size();
}
