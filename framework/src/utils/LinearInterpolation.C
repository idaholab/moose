//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearInterpolation.h"

#include "DualRealOps.h"
#include "ChainedReal.h"

#include <cassert>
#include <fstream>
#include <stdexcept>

LinearInterpolation::LinearInterpolation(const std::vector<Real> & x,
                                         const std::vector<Real> & y,
                                         const bool extrap)
  : _x(x), _y(y), _extrap(extrap)
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

template <typename T>
T
LinearInterpolation::sample(const T & x) const
{
  // this is a hard error
  if (_x.empty())
    mooseError("Trying to evaluate an empty LinearInterpolation");

  // special case for single function nodes
  if (_x.size() == 1)
    return _y[0];

  // endpoint cases
  if (_extrap)
  {
    if (x < _x[0])
      return _y[0] + (x - _x[0]) / (_x[1] - _x[0]) * (_y[1] - _y[0]);
    if (x >= _x.back())
      return _y.back() +
             (x - _x.back()) / (_x[_x.size() - 2] - _x.back()) * (_y[_y.size() - 2] - _y.back());
  }
  else
  {
    if (x < _x[0])
      return _y[0];
    if (x >= _x.back())
      return _y.back();
  }

  auto upper = std::upper_bound(_x.begin(), _x.end(), x);
  int i = std::distance(_x.begin(), upper) - 1;
  return _y[i] + (_y[i + 1] - _y[i]) * (x - _x[i]) / (_x[i + 1] - _x[i]);

  // If this point is reached, x must be a NaN.
  mooseException("Sample point in LinearInterpolation is a NaN.");
}

template Real LinearInterpolation::sample<Real>(const Real &) const;
template ADReal LinearInterpolation::sample<ADReal>(const ADReal &) const;
template ChainedReal LinearInterpolation::sample<ChainedReal>(const ChainedReal &) const;

template <typename T>
T
LinearInterpolation::sampleDerivative(const T & x) const
{
  // endpoint cases
  if (_extrap)
  {
    if (x <= _x[0])
      return (_y[1] - _y[0]) / (_x[1] - _x[0]);
    if (x >= _x.back())
      return (_y[_y.size() - 2] - _y.back()) / (_x[_x.size() - 2] - _x.back());
  }
  else
  {
    if (x < _x[0])
      return 0.0;
    if (x >= _x.back())
      return 0.0;
  }

  auto upper = std::upper_bound(_x.begin(), _x.end(), x);
  int i = std::distance(_x.begin(), upper) - 1;
  return (_y[i + 1] - _y[i]) / (_x[i + 1] - _x[i]);

  // If this point is reached, x must be a NaN.
  mooseException("Sample point in LinearInterpolation is a NaN.");
}

template Real LinearInterpolation::sampleDerivative<Real>(const Real &) const;
template ADReal LinearInterpolation::sampleDerivative<ADReal>(const ADReal &) const;
template ChainedReal LinearInterpolation::sampleDerivative<ChainedReal>(const ChainedReal &) const;

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
LinearInterpolation::getSampleSize() const
{
  return _x.size();
}
