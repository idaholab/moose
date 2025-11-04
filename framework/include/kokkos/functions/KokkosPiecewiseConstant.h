//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosPiecewiseTabularBase.h"
#include "KokkosUtils.h"

/**
 * Function which provides a piecewise constant interpolation of a provided (x,y) point data set.
 */
class KokkosPiecewiseConstant : public KokkosPiecewiseTabularBase
{
public:
  static InputParameters validParams();

  KokkosPiecewiseConstant(const InputParameters & parameters);

  using Real3 = Moose::Kokkos::Real3;

  KOKKOS_FUNCTION Real value(Real t, Real3 p) const;
  KOKKOS_FUNCTION Real integral() const;
  KOKKOS_FUNCTION Real average() const;

private:
  /// Enum for which direction to apply values
  const enum class Direction { LEFT, RIGHT, LEFT_INCLUSIVE, RIGHT_INCLUSIVE } _direction;
};

KOKKOS_FUNCTION inline Real
KokkosPiecewiseConstant::value(Real t, Real3 p) const
{
  using Moose::Kokkos::Utils::sign;

  const Real x = _has_axis ? p(_axis) : t;

  const auto len = functionSize();
  constexpr Real tolerance = 1.0e-14;

  // endpoint cases
  if ((_direction == Direction::LEFT && x < (1 + tolerance * sign(domain(0))) * domain(0)) ||
      (_direction == Direction::RIGHT && x < (1 - tolerance * sign(domain(0))) * domain(0)) ||
      (_direction == Direction::LEFT_INCLUSIVE &&
       x < (1 - tolerance * sign(domain(0))) * domain(0)) ||
      (_direction == Direction::RIGHT_INCLUSIVE &&
       x < (1 + tolerance * sign(domain(0))) * domain(0)))
    return _scale_factor * range(0);
  else if ((_direction == Direction::LEFT &&
            x > (1 + tolerance * sign(domain(len - 1))) * domain(len - 1)) ||
           (_direction == Direction::RIGHT &&
            x > (1 - tolerance * sign(domain(len - 1))) * domain(len - 1)) ||
           (_direction == Direction::LEFT_INCLUSIVE &&
            x > (1 - tolerance * sign(domain(len - 1))) * domain(len - 1)) ||
           (_direction == Direction::RIGHT_INCLUSIVE &&
            x > (1 + tolerance * sign(domain(len - 1))) * domain(len - 1)))
    return _scale_factor * range(len - 1);

  for (unsigned int i = 1; i < len; ++i)
  {
    if (_direction == Direction::LEFT && x < (1 + tolerance * sign(domain(i))) * domain(i))
      return _scale_factor * range(i - 1);
    else if (_direction == Direction::LEFT_INCLUSIVE &&
             x < (1 - tolerance * sign(domain(i))) * domain(i))
      return _scale_factor * range(i - 1);
    else if ((_direction == Direction::RIGHT && x < (1 - tolerance * sign(domain(i))) * domain(i)))
      return _scale_factor * range(i);
    else if ((_direction == Direction::RIGHT_INCLUSIVE &&
              x < (1 + tolerance * sign(domain(i))) * domain(i)))
      return _scale_factor * range(i);
  }

  return 0.0;
}

KOKKOS_FUNCTION inline Real
KokkosPiecewiseConstant::integral() const
{
  const auto len = functionSize();

  unsigned int offset = 0;

  if (_direction == Direction::RIGHT || _direction == Direction::RIGHT_INCLUSIVE)
    offset = 1;

  Real sum = 0;

  for (unsigned int i = 0; i < len - 1; ++i)
    sum += range(i + offset) * (domain(i + 1) - domain(i));

  return _scale_factor * sum;
}

KOKKOS_FUNCTION inline Real
KokkosPiecewiseConstant::average() const
{
  return integral() / (domain(functionSize() - 1) - domain(0));
}
