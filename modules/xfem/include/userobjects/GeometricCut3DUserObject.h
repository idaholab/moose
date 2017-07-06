/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef GEOMETRIC_CUT_3D_USEROBJECT_H
#define GEOMETRIC_CUT_3D_USEROBJECT_H

#include "GeometricCutUserObject.h"

using namespace libMesh;

class GeometricCut3DUserObject : public GeometricCutUserObject
{
public:
  GeometricCut3DUserObject(const InputParameters & parameters);
  virtual ~GeometricCut3DUserObject();

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
  Point _center;
  Point _normal;

  virtual bool intersectWithEdge(const Point & p1, const Point & p2, Point & pint) const;

  virtual bool isInsideCutPlane(Point p) const = 0;

  bool isInsideEdge(const Point & p1, const Point & p2, const Point & p) const;

  Real getRelativePosition(const Point & p1, const Point & p2, const Point & p) const;
};

#endif // GEOMETRIC_CUT_3D_USEROBJECT_H
