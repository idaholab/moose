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
  virtual const std::vector<Point>
  getCrackFrontPoints(unsigned int num_crack_front_points) const override;

  /** get a set of normal vectors along a crack front from a XFEM GeometricCutUserObject
   * CrackFrontDefinition wants the normal so this implementation of getCrackPlaneNormals
   * gives the CrackFrontDefinition a normal for a line element with a tangent direction
   * in the [001] direction.
   * @return A vector which contains all crack front normals
   */
  virtual const std::vector<RealVectorValue>
  getCrackPlaneNormals(unsigned int num_crack_front_points) const override;

  MeshBase & getCutterMesh() const;

protected:
  /// The FE solution mesh
  MooseMesh & _mesh;

  /// The xfem cutter mesh
  std::unique_ptr<MeshBase> _cutter_mesh;

  /// bool to specify if _cutter_mesh has been modified by crack growth
  bool wasCutterMeshModified() const { return !_active_front_node_growth_vectors.empty(); }

  /**
   * This vector of pairs orders crack tips to make the order used in this class the same as those
   * for the fracture integrals vectorpostProcessorsin created by CrackFrontDefinition.
   * The original crack front node ids found in the cutter mesh are put in pair.first and the
   * assosciated current crack front node id that grew from teh original crack front node id is in
   * pair.second.  This vector is sorted on pair.first which makes the ordering of this vector the
   * same as that used in the CrackFrontDefinition
   */
  std::vector<std::pair<dof_id_type, dof_id_type>> _original_and_current_front_node_ids;

  /// contains the active node ids and their growth vectors
  std::vector<std::pair<dof_id_type, Point>> _active_front_node_growth_vectors;

  /**
  Find growth direction at each active node
  */
  virtual void findActiveBoundaryGrowth() = 0;

  /**
    Find the original crack front nodes in the cutter mesh and use to populate
    _original_and_current_front_node_ids.
   */
  void findOriginalCrackFrontNodes();

  /// grow the cutter mesh
  void growFront();
};
