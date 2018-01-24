//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LineSegment.h"

#include "libmesh/plane.h"
#include "libmesh/vector_value.h"

LineSegment::LineSegment(const Point & p0, const Point & p1) : _p0(p0), _p1(p1) {}

bool
LineSegment::closest_point(const Point & p, bool clamp_to_segment, Point & closest_p) const
{
  Point p0_p = p - _p0;
  Point p0_p1 = _p1 - _p0;
  Real p0_p1_2 = p0_p1.norm_sq();
  Real perp = p0_p(0) * p0_p1(0) + p0_p(1) * p0_p1(1) + p0_p(2) * p0_p1(2);
  Real t = perp / p0_p1_2;
  bool on_segment = true;

  if (t < 0.0 || t > 1.0)
    on_segment = false;

  if (clamp_to_segment)
  {
    if (t < 0.0)
      t = 0.0;
    else if (t > 1.0)
      t = 1.0;
  }

  closest_p = _p0 + p0_p1 * t;
  return on_segment;
}

Point
LineSegment::closest_point(const Point & p) const
{
  Point closest_p;
  closest_point(p, true, closest_p);
  return closest_p;
}

bool
LineSegment::closest_normal_point(const Point & p, Point & closest_p) const
{
  return closest_point(p, false, closest_p);
}

bool
LineSegment::contains_point(const Point & p) const
{
  Point closest_p;
  return closest_point(p, false, closest_p) && closest_p.absolute_fuzzy_equals(p);
}

bool
LineSegment::intersect(const Plane & pl, Point & intersect_p) const
{
  /**
   * There are three cases in 3D for intersection of a line and a plane
   * Case 1: The line is parallel to the plane - No intersection
   *         Numerator = non-zero
   *         Denominator = zero
   *
   * Case 2: The line is within the plane - Inf intersection
   *         Numerator = zero
   *         Denominator = zero
   *
   * Case 3: The line intersects the plane at a single point
   *         Denominator = non-zero
   */

  Point pl0 = pl.get_planar_point();
  RealVectorValue N = pl.unit_normal(_p0);
  RealVectorValue I = (_p1 - _p0).unit();

  Real numerator = (pl0 - _p0) * N;
  Real denominator = I * N;

  // The Line is parallel to the plane
  if (std::abs(denominator) < 1.e-10)
  {
    // The Line is on the plane
    if (std::abs(numerator) < 1.e-10)
    {
      // The solution is not unique so we'll just pick an end point for now
      intersect_p = _p0;
      return true;
    }
    return false;
  }

  Real d = numerator / denominator;

  // Make sure we haven't moved off the line segment!
  if (d + libMesh::TOLERANCE < 0 || d - libMesh::TOLERANCE > (_p1 - _p0).norm())
    return false;

  intersect_p = d * I + _p0;

  return true;
}

bool
LineSegment::intersect(const LineSegment & l, Point & intersect_p) const
{
  /**
   * First check for concurance:
   *
   *
   * | x1 y1 z1 1 |
   * | x2 y2 z2 1 | = (x3 - x1) * [(x2-x1) x (x4-x3)] = 0
   * | x3 y3 z3 1 |
   * | x4 y4 z4 1 |
   *
   *
   * Solve:
   *   x = _p0 + (_p1 - _p0)*s
   *   x = l.p0 + (l._p1 - l.p0)*t
   *
   *   where
   *   a = _p1 - _p0
   *   b = l._p1 - l._p0
   *   c = l._p0 - _p0
   *
   *   s = (c x b) * (a x b) / | a x b |^2
   */
  RealVectorValue a = _p1 - _p0;
  RealVectorValue b = l._p1 - l._p0;
  RealVectorValue c = l._p0 - _p0;

  RealVectorValue v = a.cross(b);

  // Check for parallel lines
  if (std::abs(v.norm()) < 1.e-10 && std::abs(c.cross(a).norm()) < 1.e-10)
  {
    // TODO: The lines are co-linear but more work is needed to determine and intersection point
    //       it could be the case that the line segments don't lie on the same line or overlap only
    //       a bit
    return true;
  }

  // Check that the lines are coplanar
  Real concur = c * (a.cross(b));
  if (std::abs(concur) > 1.e-10)
    return false;

  Real s = (c.cross(b) * v) / (v * v);
  Real t = (c.cross(a) * v) / (v * v);

  // if s and t are between 0 and 1 then the Line Segments intersect
  // TODO: We could handle other case of clamping to the end of Line
  //       Segements if we want to here

  if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
  {
    intersect_p = _p0 + s * a;
    return true;
  }
  return false;

  /**
   * Parameteric Equation of lines
   * _p0 + t(v0) = l._p0 + u(v1)
   *
   * Case 1: Parallel Lines
   *         v0 x v1 == 0
   *
   * Case 1a: Collinear Lines
   *         v0 x v1 == 0
   *         (l._p0 - _p0) x (_p1 - _p0) == 0
   *
   * Case 2: Intersecting Lines
   *         0 <= t <= 1
   *         0 <= u <= 1
   *
   *
   * Case 1: The lines do not intersect
   *         vleft cross vright = non-zero
   *
   * Case 2: The lines are co-linear
   *         vleft cross vright = zero
   *         vleft (Denominator) = zero
   *
   * Case 3: The line intersect at a single point
   *         vleft cross vright = zero
   *         vleft (Denominator) = non-zero
  RealVectorValue v0 = _p1 - _p0;
  RealVectorValue v1 = l._p1 - l._p0;
  RealVectorValue v2 = l._p0 - _p0;

  RealVectorValue vbot = v0.cross(v1);
  RealVectorValue vtop = v2.cross(v1);

  RealVectorValue crossed = vleft.cross(vright);

  // Case 1: No intersection
  if (std::abs(vleft.cross(vright).size()) > 1.e-10)
    return false;

  // Case 2: Co-linear (just return one of the end points)
  if (std::abs(vleft.size()) < 1.e-10)
  {
    intersect_p = _p0;
    return true;
  }

  // Case 3:

  //TODO: We could detect whether the Line Segments actually overlap
  //      instead of whether the Lines are co-linear

  Real a = vright.size()/vleft.size();
  intersect_p = _p0 + a*v0;
  return true;
     */
}
