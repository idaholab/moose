//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearInterpolation.h"

#include <cassert>
#include <fstream>
#include <stdexcept>

int LinearInterpolation::_file_number = 0;

LinearInterpolation::LinearInterpolation(const std::vector<Real> & x, const std::vector<Real> & y)
  : _x(x), _y(y)
{
  errorCheck();
}

void
LinearInterpolation::errorCheck()
{
  if (_x.size() != _y.size())
    throw std::domain_error("Vectors are not the same length");

  for (unsigned int i = 0; i + 1 < _x.size(); ++i)
    if (_x[i] >= _x[i + 1])
    {
      std::ostringstream oss;
      oss << "x-values are not strictly increasing: x[" << i << "]: " << _x[i] << " x[" << i + 1
          << "]: " << _x[i + 1];
      throw std::domain_error(oss.str());
    }
}

Real
LinearInterpolation::sample(Real x) const
{
  // sanity check (empty LinearInterpolations get constructed in many places
  // so we cannot put this into the errorCheck)
  assert(_x.size() > 0);

  // endpoint cases
  if (x <= _x[0])
    return _y[0];
  if (x >= _x.back())
    return _y.back();

  for (unsigned int i = 0; i + 1 < _x.size(); ++i)
    if (x >= _x[i] && x < _x[i + 1])
      return _y[i] + (_y[i + 1] - _y[i]) * (x - _x[i]) / (_x[i + 1] - _x[i]);

  throw std::out_of_range("Unreachable");
  return 0;
}

Real
LinearInterpolation::sampleDerivative(Real x) const
{
  // endpoint cases
  if (x < _x[0])
    return 0.0;
  if (x >= _x[_x.size() - 1])
    return 0.0;

  for (unsigned int i = 0; i + 1 < _x.size(); ++i)
    if (x >= _x[i] && x < _x[i + 1])
      return (_y[i + 1] - _y[i]) / (_x[i + 1] - _x[i]);

  throw std::out_of_range("Unreachable");
  return 0;
}

Real
LinearInterpolation::integrate()
{
  Real answer = 0;
  for (unsigned int i = 1; i < _x.size(); ++i)
    answer += 0.5 * (_y[i] + _y[i - 1]) * (_x[i] - _x[i - 1]);

  return answer;
}

Real
LinearInterpolation::domain(int i) const
{
  return _x[i];
}

Real
LinearInterpolation::range(int i) const
{
  return _y[i];
}

unsigned int
LinearInterpolation::getSampleSize()
{
  return _x.size();
}
