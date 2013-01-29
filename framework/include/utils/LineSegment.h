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

#ifndef LINESEGMENT_H
#define LINESEGMENT_H

// libMesh includes
#include "point.h"

// forward declares
namespace libMesh {
  class Plane;
}


class LineSegment
{
 public:

  LineSegment (const Point & p0, const Point & p1);

  virtual ~LineSegment();

  /**
   * Returns the closest point on the LineSegment
   * to the passed in point.  Note that the closest point may be
   * one of the ends of the LineSegment.
   */
  Point closest_point (const Point & p) const;

  /**
   * Finds the closest point on the Line determined by the
   * Line Segments.  Returns a boolean indicating whether
   * that normal point is within the LineSegment or not
   */
  bool closest_normal_point (const Point & p, Point & closest_p) const;

  /**
   * Determines whether a point is in a line segment or not
   */
  bool contains_point (const Point & p) const;

  bool intersect (const Plane & pl, Point & intersect_p) const;

  bool intersect (const LineSegment & l1, Point & intersect_p) const;

 private:
  bool closest_point(const Point & p, bool clamp_to_segment, Point & closest_p) const;

  Point _p0, _p1;
};

#endif // LINESEGMENT_H
