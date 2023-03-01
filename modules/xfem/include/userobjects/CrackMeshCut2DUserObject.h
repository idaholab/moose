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

class Function;

/**
 * CrackMeshCut2DUserObject: (1) reads in a mesh describing the crack surface,
 * (2) uses the mesh to do initial cutting of 2D elements, and
 * (3) grows the mesh based on prescribed growth functions.
 */

class CrackMeshCut2DUserObject : public GeometricCutUserObject
{
public:
  static InputParameters validParams();

  CrackMeshCut2DUserObject(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void initialize() override;

  virtual const std::vector<Point>
  getCrackFrontPoints(unsigned int num_crack_front_points) const override;
  virtual const std::vector<RealVectorValue>
  getCrackPlaneNormals(unsigned int num_crack_front_points) const override;

  virtual bool cutElementByGeometry(const Elem * elem,
                                    std::vector<Xfem::CutEdge> & cut_edges,
                                    std::vector<Xfem::CutNode> & cut_nodes) const override;
  virtual bool cutElementByGeometry(const Elem * elem,
                                    std::vector<Xfem::CutFace> & cut_faces) const override;
  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_edges,
                                     std::vector<Xfem::CutEdge> & cut_edges) const override;
  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_faces,
                                     std::vector<Xfem::CutFace> & cut_faces) const override;

  /**
    Find all active boundary nodes in the cutter mesh
    Find boundary nodes that will grow; nodes outside of the structural mesh are inactive
    called by paris law
   */
  void findActiveBoundaryNodes();

  MeshBase & getCutterMesh() const;

protected:
  /// The cutter mesh
  std::unique_ptr<MeshBase> _cutter_mesh;

  /// The structural mesh
  MooseMesh & _mesh;

  /// Enum to for crack growth direction
  enum class GrowthDirectionEnum
  {
    FUNCTION
  };
  /// The direction method for growing mesh at the front
  const GrowthDirectionEnum _growth_dir_method;

  /// Enum to for crack growth speed
  enum class GrowthSpeedEnum
  {
    FUNCTION
  };
  /// The speed method for growing mesh at the front
  const GrowthSpeedEnum _growth_speed_method;

  /// Number of steps to grow the mesh
  unsigned int _n_step_growth;

  /// Boundary nodes of the cutter mesh
  std::vector<dof_id_type> _boundary_node_ids;

  /// Active boundary nodes where growth is allowed
  std::vector<dof_id_type> _active_boundary_node_ids;

  /// Inactive boundary nodes outside mooseMesh
  std::vector<dof_id_type> _inactive_boundary_node_ids;

  /// Growth direction for active boundaries
  std::vector<Point> _active_node_id_direction;

  /// New boundary after growth
  std::vector<dof_id_type> _active_boundary_node_front_ids;

  /**
    Find boundary nodes of the cutter mesh
    This is a simple algorithm simply based on the added angle = 360 degrees
    Works fine for planar cutting surface for curved cutting surface, need to re-work this
    subroutine to make it more general
   */
  void findBoundaryNodes();

  /**
    Find growth direction at each active node
   */
  void findActiveBoundaryDirection();

  /**
    Grow the cutter mesh
   */
  void growFront();

  /**
    Parsed functions of front growth
   */
  const Function * _func_x;
  const Function * _func_y;
  const Function * _func_v;

  /// Gets the time of the previous call to this user object.
  /// This is used to determine if certain things should be executed or not
  Real _time_of_previous_call_to_UO;
};
