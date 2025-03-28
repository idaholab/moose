//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceMeshCutUserObjectBase.h"
/**
 * Mesh cutter for 2D material interface problems.
 */

class XFEMMovingInterfaceVelocityBase;

class InterfaceMeshCut2DUserObject : public InterfaceMeshCutUserObjectBase
{
public:
  static InputParameters validParams();

  InterfaceMeshCut2DUserObject(const InputParameters & parameters);

  virtual bool cutElementByGeometry(const Elem * elem,
                                    std::vector<Xfem::CutEdge> & cut_edges,
                                    std::vector<Xfem::CutNode> & cut_nodes) const override;
  virtual bool cutElementByGeometry(const Elem * elem,
                                    std::vector<Xfem::CutFace> & cut_faces) const override;
  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_edges,
                                     std::vector<Xfem::CutEdge> & cut_edges) const override;
  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_faces,
                                     std::vector<Xfem::CutFace> & cut_faces) const override;

  virtual Real calculateSignedDistance(Point p) const override;

  virtual Point nodeNormal(const unsigned int & node_id) override;

  virtual void calculateNormals() override;

protected:
  /// Map of element normal
  std::unordered_map<unsigned int, Point> _element_normals;
};
