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
#include "CrackFrontDefinition.h"

#include <array>

class Function;

/**
 * CrackMeshCut3DUserObject: (1) reads in a mesh describing the crack surface,
 * (2) uses the mesh to do initial cutting of 3D elements, and
 * (3) grows the mesh based on prescribed growth functions.
 */

class CrackMeshCut3DUserObject : public GeometricCutUserObject
{
public:
  static InputParameters validParams();

  CrackMeshCut3DUserObject(const InputParameters & parameters);

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
   */
  void findActiveBoundaryNodes();

  /**
    Get crack front points in the active segment
    -1 means inactive; positive is the point's index in the Crack Front Definition starting from 0
   */
  std::vector<int> getFrontPointsIndex();

  /**
    Return growth size at the active boundary to the mesh cutter
   */
  void setSubCriticalGrowthSize(std::vector<Real> & growth_size);

  /**
    Return the total number of crack front points.
    This function is currently not called anywhere in the code.
    Ideally, in a future update, the number of crack front points will be managed by
    CrackFrontPointsProvider instead of CrackFrontDefinition. In that case,
    getNumberOfCrackFrontPoints() defined here may be used to override a virtual function defined in
    CrackFrontPointsProvider
   */
  unsigned int getNumberOfCrackFrontPoints() const;

protected:
  /// The cutter mesh
  std::unique_ptr<MeshBase> _cut_mesh;

  /// The cutter mesh has triangluar elements only
  const unsigned int _cut_elem_nnode = 3;
  const unsigned int _cut_elem_dim = 2;

  /// The structural mesh
  MooseMesh & _mesh;

  /// The crack front definition
  CrackFrontDefinition * _crack_front_definition;

  /// updated crack front definition
  /// they are in the same order as defined in the input but the number of nodes may increase
  /// its difference from _front is that: _front does not necessarily follow the order of crack front definition
  /// therefore, _crack_front_points is generated from _front with the order of crack front definition
  /// limitation: this approach does not currently support the growth of one crack front into two
  std::vector<dof_id_type> _crack_front_points;

  /// The direction method for growing mesh at the front
  std::string _growth_dir_method;

  /// The speed method for growing mesh at the front
  std::string _growth_speed_method;

  /// The structural mesh must be 3D only
  const unsigned int _elem_dim = 3;

  /// Used to define intersection points
  const Real _const_intersection = 0.01;

  /// Used for cutter mesh refinement and front advancement
  Real _size_control;

  /// Number of steps to grow the mesh
  unsigned int _n_step_growth;

  /// Variables to help control the work flow
  bool _stop;
  bool _grow;

  /// Boundary nodes of the cutter mesh
  std::vector<dof_id_type> _boundary;

  /// Active boundary nodes where growth is allowed
  std::vector<std::vector<dof_id_type>> _active_boundary;

  /// Inactive boundary
  std::vector<unsigned int> _inactive_boundary_pos;

  /// Front nodes that are grown from the crack front definition defined in the input
  /// therefore, they are (1) in the same order as defined in the input and (2) the number of nodes does not change
  std::vector<dof_id_type> _tracked_crack_front_points;

  bool _cfd;

  /// Edges at the boundary
  std::set<Xfem::CutEdge> _boundary_edges;

  /// A map of boundary nodes and their neighbors
  std::map<dof_id_type, std::vector<dof_id_type>> _boundary_map;

  /// Growth direction for active boundaries
  std::vector<std::vector<Point>> _active_direction;

  /// Growth size for the active boundary in a subcritical simulation
  std::vector<Real> _growth_size;

  /// Fatigue life
  std::vector<unsigned long int> _dn;
  std::vector<unsigned long int> _n;

  /// New boundary after growth
  std::vector<std::vector<dof_id_type>> _front;

  /// Indicator that shows if the cutting mesh is modified or not in this calculation step
  bool _is_mesh_modified;

  /// Total number of crack front points in the mesh cutter
  unsigned int _num_crack_front_points;

  /**
    Check if a line intersects with an element
   */
  virtual bool intersectWithEdge(const Point & p1,
                                 const Point & p2,
                                 const std::vector<Point> & _vertices,
                                 Point & point) const;

  /**
    Find directional intersection along the positive extension of the vector from p1 to p2
   */
  bool findIntersection(const Point & p1,
                        const Point & p2,
                        const std::vector<Point> & vertices,
                        Point & point) const;

  /**
    Check if point p is inside the edge p1-p2
   */
  bool isInsideEdge(const Point & p1, const Point & p2, const Point & p) const;

  /**
    Get the relative position of p from p1
   */
  Real getRelativePosition(const Point & p1, const Point & p2, const Point & p) const;

  /**
    Check if point p is inside a plane
   */
  bool isInsideCutPlane(const std::vector<Point> & _vertices, const Point & p) const;

  /**
    Find boundary nodes of the cutter mesh
    This is a simple algorithm simply based on the added angle = 360 degrees
    Works fine for planar cutting surface for curved cutting surface, need to re-work this
    subroutine to make it more general
   */
  void findBoundaryNodes();

  /**
    Find boundary edges of the cutter mesh
   */
  void findBoundaryEdges();

  /**
    Sort boundary nodes to be in the right order along the boundary
   */
  void sortBoundaryNodes();

  /**
    Find distance between two nodes
   */
  Real findDistance(dof_id_type node1, dof_id_type node2);

  /**
    If boundary nodes are too sparse, add nodes in between
   */
  void refineBoundary();

  /**
    Find growth direction at each active node
   */
  void findActiveBoundaryDirection();

  /**
    Grow the cutter mesh
   */
  void growFront();

  /**
    Sort the front nodes
   */
  void sortFrontNodes();

  /**
    Find front-structure intersections
   */
  void findFrontIntersection();

  /**
    Refine the mesh at the front
   */
  void refineFront();

  /**
    Create tri3 elements between the new front and the old front
   */
  void triangulation();

  /**
    Join active boundaries and inactive boundaries to be the new boundary
   */
  void joinBoundary();

  /**
    Parsed functions of front growth
   */
  const Function * _func_x;
  const Function * _func_y;
  const Function * _func_z;
  const Function * _func_v;
};
