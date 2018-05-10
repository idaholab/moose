//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef GEOMETRICCUT2DUSEROBJECT_H
#define GEOMETRICCUT2DUSEROBJECT_H

#include "GeometricCutUserObject.h"

// Forward declarations
class GeometricCut2DUserObject;

template <>
InputParameters validParams<GeometricCut2DUserObject>();

class GeometricCut2DUserObject : public GeometricCutUserObject
{
public:
  GeometricCut2DUserObject(const InputParameters & parameters);

  virtual bool cutElementByGeometry(const Elem * elem,
                                    std::vector<Xfem::CutEdge> & cut_edges,
                                    std::vector<Xfem::CutNode> & cut_nodes,
                                    Real time) const override;
  virtual bool cutElementByGeometry(const Elem * elem,
                                    std::vector<Xfem::CutFace> & cut_faces,
                                    Real time) const override;

  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_edges,
                                     std::vector<Xfem::CutEdge> & cut_edges,
                                     Real time) const override;
  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_faces,
                                     std::vector<Xfem::CutFace> & cut_faces,
                                     Real time) const override;

protected:
  std::vector<std::pair<Point, Point>> _cut_line_endpoints;

  bool IntersectSegmentWithCutLine(const Point & segment_point1,
                                   const Point & segment_point2,
                                   const std::pair<Point, Point> & cutting_line_points,
                                   const Real & cutting_line_fraction,
                                   Real & segment_intersection_fraction) const;

  Real crossProduct2D(const Point & point_a, const Point & point_b) const;

  /**
   * Find the fractional distance along a specified cut line for the current time
   * that is currently active. Used for time-based propagation along a line
   * @param cut_num Index of the cut being queried
   * @param time      Current simulation time
   * @return Current fractional distance
   */
  virtual Real cutFraction(unsigned int cut_num, Real time) const;

  /// Vector of start/end times for each cut segment
  std::vector<std::pair<Real, Real>> _cut_time_ranges;
};

#endif // GEOMETRICCUT2DUSEROBJECT_H
