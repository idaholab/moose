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

#include <array>

class Function;

/**
 * MeshCut3DUserObject: (1) reads in a mesh describing the crack surface,
 * (2) uses the mesh to do initial cutting of 3D elements, and
 * (3) grows the mesh based on prescribed growth functions.
 */

class MeshCut3DUserObject : public GeometricCutUserObject
{
public:
  static InputParameters validParams();

  MeshCut3DUserObject(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void initialize() override;
  virtual const std::vector<Point>
  getCrackFrontPoints(unsigned int num_crack_front_points) const override;

  virtual bool cutElementByGeometry(const Elem * elem,
                                    std::vector<Xfem::CutEdge> & cut_edges,
                                    std::vector<Xfem::CutNode> & cut_nodes) const override;
  virtual bool cutElementByGeometry(const Elem * elem,
                                    std::vector<Xfem::CutFace> & cut_faces) const override;
  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_edges,
                                     std::vector<Xfem::CutEdge> & cut_edges) const override;
  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_faces,
                                     std::vector<Xfem::CutFace> & cut_faces) const override;

protected:
  /// The cutter mesh
  std::unique_ptr<MeshBase> _cut_mesh;

  /// The cutter mesh has triangluar elements only
  const unsigned int _cut_elem_nnode = 3;
  const unsigned int _cut_elem_dim = 2;

  /// The structural mesh
  MooseMesh & _mesh;

  /// The structural mesh must be 3D only
  const unsigned int _elem_dim = 3;

  /// Used to define intersection points
  const Real _const_intersection = 0.01;

  /// Used for cutter mesh refinement and front advancement
  Real _size_control;

  /// Number of steps to grow the mesh
  unsigned int _n_step_growth;

  /// variables to help control the work flow
  bool _stop;
  bool _grow;

  /// Boundary nodes of the cutter mesh
  std::vector<dof_id_type> _boundary;

  /// Edges at the boundary
  std::set<Xfem::CutEdge> _boundary_edges;

  /// A map of boundary nodes and their neighbors
  std::map<dof_id_type, std::vector<dof_id_type>> _boundary_map;

  /// Active boundary nodes where growth is allowed
  std::vector<std::vector<dof_id_type>> _active_boundary;

  /// Growth direction for active boundaries
  std::vector<std::vector<Point>> _active_direction;

  /// Inactive boundary
  std::vector<unsigned int> _inactive_boundary_pos;

  /// New boundary after growth
  std::vector<std::vector<dof_id_type>> _front;

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
    Find all active boundary nodes in the cutter mesh
    Find boundary nodes that will grow; nodes outside of the structural mesh are inactive
   */
  void findActiveBoundaryNodes();

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
  const Function & _func_x;
  const Function & _func_y;
  const Function & _func_z;
};
