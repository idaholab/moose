/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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

  virtual bool active(Real time) const override;

  virtual bool cutElementByGeometry(const Elem * elem,
                                    std::vector<CutEdge> & cut_edges,
                                    std::vector<CutNode> & cut_nodes,
                                    Real time) const override;
  virtual bool cutElementByGeometry(const Elem * elem,
                                    std::vector<CutFace> & cut_faces,
                                    Real time) const override;

  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_edges,
                                     std::vector<CutEdge> & cut_edges,
                                     Real time) const override;
  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_faces,
                                     std::vector<CutFace> & cut_faces,
                                     Real time) const override;

protected:
  std::vector<std::pair<Point, Point>> _cut_line_endpoints;

  bool IntersectSegmentWithCutLine(const Point & segment_point1,
                                   const Point & segment_point2,
                                   const std::pair<Point, Point> & cutting_line_points,
                                   const Real & cutting_line_fraction,
                                   Real & segment_intersection_fraction) const;

  Real crossProduct2D(const Point & point_a, const Point & point_b) const;
};

#endif // GEOMETRICCUT2DUSEROBJECT_H
