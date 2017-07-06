/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef GEOMETRIC_CUT_2D_USEROBJECT_H
#define GEOMETRIC_CUT_2D_USEROBJECT_H

#include "GeometricCutUserObject.h"

// Forward declarations
class GeometricCut2DUserObject;

template <>
InputParameters validParams<GeometricCut2DUserObject>();

class GeometricCut2DUserObject : public GeometricCutUserObject
{
public:
  GeometricCut2DUserObject(const InputParameters & parameters);
  ~GeometricCut2DUserObject();

  virtual void initialize(){};
  virtual void execute(){};
  virtual void finalize(){};

  virtual bool active(Real time) const;

  virtual bool
  cutElementByGeometry(const Elem * elem, std::vector<CutEdge> & cut_edges, Real time) const;
  virtual bool
  cutElementByGeometry(const Elem * elem, std::vector<CutFace> & cut_faces, Real time) const;

  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_edges,
                                     std::vector<CutEdge> & cut_edges,
                                     Real time) const;
  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_faces,
                                     std::vector<CutFace> & cut_faces,
                                     Real time) const;

protected:
  std::vector<Point> _cut_line_start_points;
  std::vector<Point> _cut_line_end_points;

  bool IntersectSegmentWithCutLine(const Point & segment_point1,
                                   const Point & segment_point2,
                                   const Point & cutting_line_point1,
                                   const Point & cutting_line_point2,
                                   const Real & cutting_line_fraction,
                                   Real & segment_intersection_fraction) const;
  Real crossProduct2D(Real ax, Real ay, Real bx, Real by) const;
};

#endif // GEOMETRIC_CUT_2D_USEROBJECT_H
