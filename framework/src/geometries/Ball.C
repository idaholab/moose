//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Ball.h"
#include "LineSegment.h"

#include <cmath>

bool
Ball::intersect(const LineSegment & line_segment) const
{
  const Point & p0 = line_segment.start();
  const Point & p1 = line_segment.end();
  const Point d = p1 - p0;
  const Point f = p0 - _c;

  const Real a = d.norm_sq();
  const Real b = 2.0 * (f * d);
  const Real c = f.norm_sq() - _r * _r;

  if (a == 0.0)
    return f.norm_sq() <= _r * _r;

  const Real discriminant = b * b - 4.0 * a * c;
  if (discriminant < 0.0)
    return false;

  const Real sqrt_disc = std::sqrt(discriminant);
  const Real inv_denom = 1.0 / (2.0 * a);
  const Real t1 = (-b - sqrt_disc) * inv_denom;
  const Real t2 = (-b + sqrt_disc) * inv_denom;

  return (t1 >= 0.0 && t1 <= 1.0) || (t2 >= 0.0 && t2 <= 1.0);
}

Ball
Ball::computeBoundingBall() const
{
  return *this;
}
