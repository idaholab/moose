//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PiecewiseConstant.h"
#include "MathUtils.h"

registerMooseObject("MooseApp", PiecewiseConstant);

InputParameters
PiecewiseConstant::validParams()
{
  InputParameters params = PiecewiseTabularBase::validParams();
  MooseEnum direction("LEFT RIGHT LEFT_INCLUSIVE RIGHT_INCLUSIVE", "LEFT");
  params.addParam<MooseEnum>(
      "direction", direction, "Direction to look to find value: " + direction.getRawNames());
  params.addClassDescription("Defines data using a set of x-y data pairs");
  return params;
}

PiecewiseConstant::PiecewiseConstant(const InputParameters & parameters)
  : PiecewiseTabularBase(parameters),
    _direction(getParam<MooseEnum>("direction").getEnum<Direction>())
{
}

Real
PiecewiseConstant::value(Real t, const Point & p) const
{
  const Real x = _has_axis ? p(_axis) : t;

  unsigned i = 1;
  const unsigned len = functionSize();
  const Real tolerance = 1.0e-14;

  // endpoint cases
  if ((_direction == Direction::LEFT &&
       x < (1 + tolerance * MathUtils::sign(domain(0))) * domain(0)) ||
      (_direction == Direction::RIGHT &&
       x < (1 - tolerance * MathUtils::sign(domain(0))) * domain(0)) ||
      (_direction == Direction::LEFT_INCLUSIVE &&
       x < (1 - tolerance * MathUtils::sign(domain(0))) * domain(0)) ||
      (_direction == Direction::RIGHT_INCLUSIVE &&
       x < (1 + tolerance * MathUtils::sign(domain(0))) * domain(0)))
    return _scale_factor * range(0);
  else if ((_direction == Direction::LEFT &&
            x > (1 + tolerance * MathUtils::sign(domain(len - 1))) * domain(len - 1)) ||
           (_direction == Direction::RIGHT &&
            x > (1 - tolerance * MathUtils::sign(domain(len - 1))) * domain(len - 1)) ||
           (_direction == Direction::LEFT_INCLUSIVE &&
            x > (1 - tolerance * MathUtils::sign(domain(len - 1))) * domain(len - 1)) ||
           (_direction == Direction::RIGHT_INCLUSIVE &&
            x > (1 + tolerance * MathUtils::sign(domain(len - 1))) * domain(len - 1)))
    return _scale_factor * range(len - 1);

  for (; i < len; ++i)
  {
    if (_direction == Direction::LEFT &&
        x < (1 + tolerance * MathUtils::sign(domain(i))) * domain(i))
      return _scale_factor * range(i - 1);
    else if (_direction == Direction::LEFT_INCLUSIVE &&
             x < (1 - tolerance * MathUtils::sign(domain(i))) * domain(i))
      return _scale_factor * range(i - 1);
    else if ((_direction == Direction::RIGHT &&
              x < (1 - tolerance * MathUtils::sign(domain(i))) * domain(i)))
      return _scale_factor * range(i);
    else if ((_direction == Direction::RIGHT_INCLUSIVE &&
              x < (1 + tolerance * MathUtils::sign(domain(i))) * domain(i)))
      return _scale_factor * range(i);
  }

  return 0.0;
}

ADReal
PiecewiseConstant::value(const ADReal & t, const ADPoint & p) const
{
  // piecewise constant has all zero derivatives (ignoring discontinuities)
  return value(MetaPhysicL::raw_value(t), MetaPhysicL::raw_value(p));
}

Real
PiecewiseConstant::timeDerivative(Real /*t*/, const Point & /*p*/) const
{
  return 0;
}

Real
PiecewiseConstant::integral() const
{
  const unsigned len = functionSize();
  Real sum = 0;
  unsigned offset = 0;

  if (_direction == Direction::RIGHT || _direction == Direction::RIGHT_INCLUSIVE)
    offset = 1;

  for (unsigned i = 0; i < len - 1; ++i)
    sum += range(i + offset) * (domain(i + 1) - domain(i));

  return _scale_factor * sum;
}

Real
PiecewiseConstant::average() const
{
  return integral() / (domain(functionSize() - 1) - domain(0));
}
