/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef XFEM_GEOMETRIC_CUT_3D_H
#define XFEM_GEOMETRIC_CUT_3D_H

#include "XFEMGeometricCut.h"

using namespace libMesh;

class XFEMGeometricCut3D : public XFEMGeometricCut
{
public:
  XFEMGeometricCut3D(Real t0, Real t1);

  virtual ~XFEMGeometricCut3D();

  virtual bool active(Real time);

  virtual bool cutElementByGeometry(const Elem * elem, std::vector<CutEdge> & cut_edges, Real time);
  virtual bool cutElementByGeometry(const Elem * elem, std::vector<CutFace> & cut_faces, Real time);

  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_edges,
                                     std::vector<CutEdge> & cut_edges,
                                     Real time);
  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_faces,
                                     std::vector<CutFace> & cut_faces,
                                     Real time);

protected:
  Point _center;
  Point _normal;

  virtual bool intersectWithEdge(const Point & p1, const Point & p2, Point & pint);

  virtual bool isInsideCutPlane(Point p) = 0;

  bool isInsideEdge(const Point & p1, const Point & p2, const Point & p);

  Real getRelativePosition(const Point & p1, const Point & p2, const Point & p);
};

#endif
