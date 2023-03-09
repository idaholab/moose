//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeometricCutUserObject.h"

/**
 * MeshCut2DUserObjectBase: (1) reads in a mesh describing the crack surface,
 * (2) Fills xfem cut element ojbects.
 * Derived classes modify the class and grow the mesh
 */

class MeshCut2DUserObjectBase : public GeometricCutUserObject
{
public:
  static InputParameters validParams();

  MeshCut2DUserObjectBase(const InputParameters & parameters);

  virtual bool cutElementByGeometry(const Elem * elem,
                                    std::vector<Xfem::CutEdge> & cut_edges,
                                    std::vector<Xfem::CutNode> & cut_nodes) const override;
  virtual bool cutElementByGeometry(const Elem * elem,
                                    std::vector<Xfem::CutFace> & cut_faces) const override;
  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_edges,
                                     std::vector<Xfem::CutEdge> & cut_edges) const override;
  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_faces,
                                     std::vector<Xfem::CutFace> & cut_faces) const override;

  MeshBase & getCutterMesh() const;

protected:
  /// The cutter mesh
  std::unique_ptr<MeshBase> _cutter_mesh;

  /// The structural mesh
  MooseMesh & _mesh;
};
