//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshCutUserObjectBase.h"
#include "CrackFrontDefinition.h"

#include <array>

class Function;

/**
 * CrackMeshCut3DUserObject: (1) reads in a mesh describing the crack surface,
 * (2) uses the mesh to do initial cutting of 3D elements, and
 * (3) grows the mesh based on prescribed growth functions.
 */

class CrackMeshCut3DUserObject : public MeshCutUserObjectBase
{
public:
  static InputParameters validParams();

  CrackMeshCut3DUserObject(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void initialize() override;
  virtual void meshChanged() override;

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
  std::vector<int> getFrontPointsIndex() const;

  /**
    Return the total number of crack front points.
    Returns the number of crack front points for use by CrackFrontDefinition.
    Overrides the virtual function defined in CrackFrontPointsProvider.
   */
  virtual unsigned int getNumberOfCrackFrontPoints() const override;

protected:
  /// The cutter mesh has triangluar elements only
  const unsigned int _cut_elem_nnode = 3;
  const unsigned int _cut_elem_dim = 2;

  /// The structural mesh
  MooseMesh & _mesh;

  /// Pointer to PointLocatorBase object
  std::unique_ptr<PointLocatorBase> _pl;

  /// The crack front definition
  CrackFrontDefinition * _crack_front_definition;

  /// updated crack front definition
  /// they are in the same order as defined in the input but the number of nodes may increase
  /// its difference from _front is that: _front does not necessarily follow the order of crack front definition
  /// therefore, _crack_front_points is generated from _front with the order of crack front definition
  /// limitation: this approach does not currently support the growth of one crack front into two
  std::vector<dof_id_type> _crack_front_points;

  /// Enum to for crack growth direction
  enum class GrowthDirectionEnum
  {
    MAX_HOOP_STRESS,
    FUNCTION
  };
  /// The direction method for growing mesh at the front
  const GrowthDirectionEnum _growth_dir_method;

  /// Enum to for crack growth rate
  enum class GrowthRateEnum
  {
    REPORTER,
    FUNCTION
  };
  /// The growth increment method for growing mesh at the front
  const GrowthRateEnum _growth_increment_method;

  /// The structural mesh must be 3D only
  const unsigned int _elem_dim = 3;

  /// Used to define intersection points
  const Real _const_intersection = 0.01;

  /// Used for cutter mesh refinement and front advancement
  const Real _size_control;

  /// Number of steps to grow the mesh
  const unsigned int _n_step_growth;

  /// Minimum element area for crack growth elements
  const Real _min_elem_area;

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

  /// is it using the crack_front_definition
  bool _cfd;

  /// Edges at the boundary
  std::set<Xfem::CutEdge> _boundary_edges;

  /// A map of boundary nodes and their neighbors
  std::map<dof_id_type, std::vector<dof_id_type>> _boundary_map;

  /// Growth direction for active boundaries
  std::vector<std::vector<Point>> _active_direction;

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
    Compute the area of a triangle defined by three points.
    Returns true if the area is above min_elem_area.
    @param p1 first vertex of the triangle
    @param p2 second vertex of the triangle
    @param p3 third vertex of the triangle
    @return true if the triangle area exceeds the minimum threshold
   */
  bool isTriAreaAboveTol(const Point & p1, const Point & p2, const Point & p3) const;

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
    Find growth direction at each active node
   */
  void findActiveBoundaryDirection();

  /**
    Grow the cutter mesh
   */
  void growFront();

  /**
    Determine whether a front node is an inactive endpoint of an active boundary segment.
    @param front_node_index index of the node within the active boundary segment
    @param front_size number of nodes in the active boundary segment
    @return true if the node is an inactive endpoint
   */
  bool isInactiveEndpoint(unsigned int front_node_index, unsigned int front_size) const;

  /**
    Compute the crack growth increment for a front node.
    @param front_node_index index of the node within the active boundary segment
    @param front_size number of nodes in the active boundary segment
    @param front_point_index mapping from active boundary nodes to crack-front-definition points
    @return the growth increment applied to the node
   */
  Real computeGrowthIncrement(unsigned int front_node_index,
                              unsigned int front_size,
                              const std::vector<int> & front_point_index) const;

  /**
    Project an inactive endpoint back outside the structural mesh if its proposed position lands
    inside the volume.
    @param segment_index index of the active boundary segment
    @param front_node_index index of the node within the active boundary segment
    @param front_size number of nodes in the active boundary segment
    @param candidate_point proposed grown node position
    @return the adjusted point position
   */
  Point projectInteriorInactiveEndpoint(unsigned int segment_index,
                                        unsigned int front_node_index,
                                        unsigned int front_size,
                                        const Point & candidate_point) const;

  /**
    Add a grown front node while checking whether the newly formed crack-front triangles are
    degenerate.
    @param segment_index index of the active boundary segment
    @param front_node_index index of the node within the active boundary segment
    @param orig_id original boundary node id
    @param candidate_point proposed grown node position
    @param front_nodes output front-node ordering for the current active segment
    @return the id that should represent this node on the grown front
   */
  dof_id_type
  appendAdvancedFrontNodeCheckingDegenerateTriangles(unsigned int segment_index,
                                                     unsigned int front_node_index,
                                                     dof_id_type orig_id,
                                                     const Point & candidate_point,
                                                     std::vector<dof_id_type> & front_nodes);

  /**
    Update the tracked crack-front-definition node when a front node advances.
    @param orig_id original crack front node id
    @param new_id replacement node id after growth
   */
  void updateTrackedCrackFrontPoint(dof_id_type orig_id, dof_id_type new_id);

  /**
    Sort the front nodes
   */
  void sortFrontNodes();

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
   * Determine initial crack front nodes from cutter mesh
   */
  void initializeCrackFrontNodes();

  /**
    Parsed functions of front growth
   */
  const Function * _func_x;
  const Function * _func_y;
  const Function * _func_z;
  const Function * _func_v;

  /// Pointer to fracture integral ki if available
  const std::vector<Real> * const _ki_vpp;
  /// Pointer to fracture integral kii if available
  const std::vector<Real> * const _kii_vpp;
  /// Pointer to reporter with growth increment if available
  const std::vector<Real> * const _growth_inc_reporter;
};
