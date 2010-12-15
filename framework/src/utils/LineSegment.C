/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "LineSegment.h"
#include "vector_value.h"

LineSegment::LineSegment(const Point & p0, const Point & p1):
  _p0(p0),
  _p1(p1)
{}

bool
LineSegment::closest_point (const Point & p, bool clamp_to_segment, Point & closest_p) const
{
  Point p0_p  = p - _p0;
  Point p0_p1 = _p1 - _p0;
  Real p0_p1_2 = p0_p1.size_sq();
  Real perp = p0_p(0)*p0_p1(0) + p0_p(1)*p0_p1(1) + p0_p(2)*p0_p1(2);
  Real t = perp / p0_p1_2;
  bool on_segment = true;

  if (t < 0.0 || t > 1.0)
    on_segment = false;

  if (clamp_to_segment)
  {
    if (t < 0.0) t = 0.0;
    else if (t > 1.0) t = 1.0;
  }
  
  closest_p = _p0 + p0_p1 * t;
  return on_segment;
}

Point
LineSegment::closest_point (const Point & p) const
{
  Point closest_p;
  closest_point(p, true, closest_p);
  return closest_p;
}

bool
LineSegment::closest_normal_point (const Point & p, Point & closest_p) const
{
  return closest_point(p, false, closest_p);
}

bool
LineSegment::contains_point (const Point & p) const
{
  Point closest_p;
  return closest_point(p, false, closest_p);
}

bool
LineSegment::intersect (const Plane & pl, Point & intersect_p) const
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
  RealVectorValue I = (_p1-_p0).unit();

  Real numerator = (pl0-_p0)*N;
  Real denominator = I*N;

  // For now we will treat Case 2 as no intersection (not unique)
  if (std::abs(denominator) < 1.e-10)
    return false;

  // TODO: d can be used to determine whether the LineSegment intersects
  // the plane or if the Line determined by the Segment intersects the plane
  // d will fall between 0 and 1 if the intersection is within the line
  // segment
  Real d = numerator / denominator;
  intersect_p = d*I + _p0;
  
  return true;
}

bool
LineSegment::intersect (const LineSegment & l, Point & intersect_p) const
{
  /**
   * There are three cases in 3D for intersection of two lines
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
   */         
  RealVectorValue v0 = _p1 - _p0;
  RealVectorValue v1 = l._p1 - l._p0;
  RealVectorValue v2 = l._p0 - _p0;
  
  RealVectorValue vleft = v0.cross(v1);
  RealVectorValue vright = v2.cross(v1);

  // If vleft cross vright = zero || vleft is zero then there is not
  // a unique intersection point
  if (std::abs(vleft.size()) < 1.e-10 ||
      std::abs(vleft.cross(vright).size()) > 1.e-10)
    return false;

  //TODO: We could break out Case 2 differently if we want to detect
  //      co-linear lines
  //TODO: We could detect whether the Line Segments actually overlap
  //      instead of whether the Lines are co-linear

  Real a = vright.size()/vleft.size();
  intersect_p = _p0 + a*v0;
  return true;
}
