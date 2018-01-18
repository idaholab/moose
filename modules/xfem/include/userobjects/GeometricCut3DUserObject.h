/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef GEOMETRICCUT3DUSEROBJECT_H
#define GEOMETRICCUT3DUSEROBJECT_H

#include "GeometricCutUserObject.h"

using namespace libMesh;

class GeometricCut3DUserObject : public GeometricCutUserObject
{
public:
  GeometricCut3DUserObject(const InputParameters & parameters);

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
  Point _center;
  Point _normal;

  virtual bool intersectWithEdge(const Point & p1, const Point & p2, Point & pint) const;

  virtual bool isInsideCutPlane(Point p) const = 0;

  bool isInsideEdge(const Point & p1, const Point & p2, const Point & p) const;

  Real getRelativePosition(const Point & p1, const Point & p2, const Point & p) const;
};

#endif // GEOMETRICCUT3DUSEROBJECT_H
