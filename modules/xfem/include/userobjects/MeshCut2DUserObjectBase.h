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
class MeshCut2DNucleationBase;
class CrackFrontDefinition;
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

  // initialSetup needs to be called by every derived class
  virtual void initialSetup() override final;

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

  /// 2D UO for nucleating cracks
  const MeshCut2DNucleationBase * _nucleate_uo;

  /// Indicator that shows if the cutting mesh is modified or not in this calculation step
  bool _is_mesh_modified;

  /**
   * This vector of pairs orders crack tips to make the order used in this class the same as those
   * for the fracture integrals VectorPostprocessors created by CrackFrontDefinition.
   * The original crack front node ids found in the cutter mesh are put in pair.first and the
   * assosciated current crack front node id that grew from the original crack front node id is in
   * pair.second.  This vector is sorted on pair.first which makes the ordering of this vector the
   * same as that used in the CrackFrontDefinition
   */
  std::vector<std::pair<dof_id_type, dof_id_type>> _original_and_current_front_node_ids;

  /// contains the active node ids and their growth vectors
  std::vector<std::pair<dof_id_type, Point>> _active_front_node_growth_vectors;

  /// user object for communicating between solid_mechanics interaction integrals and xfem cutter mesh
  CrackFrontDefinition * _crack_front_definition;

  /**
  Find growth direction at each active node
  */
  virtual void findActiveBoundaryGrowth() = 0;

  /**
   * Find the original crack front nodes in the cutter mesh and use to populate
   * _original_and_current_front_node_ids.
   */
  void findOriginalCrackFrontNodes();

  /// grow the cutter mesh
  void growFront();

  /**
   * Calls into MeshCutNucleation UO to add cracks.
   */
  void addNucleatedCracksToMesh();

private:
  /**
   * Remove nucleated cracks that are too close too each other.  Lowest map key wins
   * @param  nucleated_elems_map  map from nucleation userObject with key for mesh element id and
   * two nodes of nucleated crack
   * @param  nucleationRadius  exclusion distance between cracks
   */
  void removeNucleatedCracksTooCloseToEachOther(
      std::map<unsigned int, std::pair<RealVectorValue, RealVectorValue>> & nucleated_elems_map,
      Real nucleationRadius);
  /**
   * Remove nucleated cracks that are too close to a pre-existing crack in the mesh.
   * @param  nucleated_elems_map  map from nucleation userObject with key for mesh element id and
   * two nodes of nucleated crack
   * @param  nucleationRadius  exclusion distance between cracks
   */
  void removeNucleatedCracksTooCloseToExistingCracks(
      std::map<unsigned int, std::pair<RealVectorValue, RealVectorValue>> & nucleated_elems_map,
      Real nucleationRadius);
};
