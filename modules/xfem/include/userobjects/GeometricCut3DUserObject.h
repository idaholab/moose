//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef GEOMETRICCUT3DUSEROBJECT_H
#define GEOMETRICCUT3DUSEROBJECT_H

#include "GeometricCutUserObject.h"

using namespace libMesh;

class GeometricCut3DUserObject;

template <>
InputParameters validParams<GeometricCut3DUserObject>();

class GeometricCut3DUserObject : public GeometricCutUserObject
{
public:
  GeometricCut3DUserObject(const InputParameters & parameters);

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
  Point _center;
  Point _normal;

  virtual bool intersectWithEdge(const Point & p1, const Point & p2, Point & pint) const;

  virtual bool isInsideCutPlane(Point p) const = 0;

  bool isInsideEdge(const Point & p1, const Point & p2, const Point & p) const;

  Real getRelativePosition(const Point & p1, const Point & p2, const Point & p) const;
};

#endif // GEOMETRICCUT3DUSEROBJECT_H
