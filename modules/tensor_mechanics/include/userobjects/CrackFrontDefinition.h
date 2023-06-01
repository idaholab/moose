//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"
#include "CrackFrontPointsProvider.h"
#include "BoundaryRestrictable.h"
#include <set>
#include "ADRankTwoTensorForward.h"

class AuxiliarySystem;

// libMesh forward declarations
namespace libMesh
{
class QBase;
}

// fixme lynn this needs to be removed from the globalname space in a seperate commit
void addCrackFrontDefinitionParams(InputParameters & params);
/**
 * Class used in fracture integrals to define geometric characteristics of the crack front
 */
class CrackFrontDefinition : public GeneralUserObject, public BoundaryRestrictable
{
public:
  static InputParameters validParams();

  CrackFrontDefinition(const InputParameters & parameters);
  virtual ~CrackFrontDefinition();

  virtual void initialSetup() override;
  virtual void initialize() override;
  virtual void finalize() override;
  virtual void execute() override;

  /**
   * Change the number of crack front nodes. As the crack grows, the number of crack fronts nodes
   * may keep increasing in many cases.
   */
  void updateNumberOfCrackFrontPoints(const std::size_t num_points);

  /**
   * Get the node pointer for a specified node on the crack front
   * @param node_index Index of the node
   * @return Pointer to node
   */
  const Node * getCrackFrontNodePtr(const std::size_t node_index) const;

  /**
   * Get a Point object for a specified point on the crack front
   * @param point_index Index of the point
   * @return Point object
   */
  const Point * getCrackFrontPoint(const std::size_t point_index) const;

  /**
   * Get the vector tangent to the crack front at a specified position
   * @param point_index Index of the point
   * @return tangent vector
   */
  const RealVectorValue & getCrackFrontTangent(const std::size_t point_index) const;

  /**
   * Get the length of the line segment on the crack front ahead of the specified position
   * @param point_index Index of the point
   * @return Line segment length
   */
  Real getCrackFrontForwardSegmentLength(const std::size_t point_index) const;

  /**
   * Get the length of the line segment on the crack front behind the specified position
   * @param point_index Index of the point
   * @return Line segment length
   */
  Real getCrackFrontBackwardSegmentLength(const std::size_t point_index) const;

  /**
   * Get the unit vector of the crack extension direction at the specified position
   * @param point_index Index of the point
   * @return Crack extension direction vector
   */
  const RealVectorValue & getCrackDirection(const std::size_t point_index) const;

  /**
   * Get the distance along the crack front from the beginning of the crack to the
   * specified position
   * @param point_index Index of the point
   * @return Distance along crack
   */
  Real getDistanceAlongFront(const std::size_t point_index) const;

  /**
   * Whether the distance along the crack front is available as an angle
   * @return true if it is available as an angle
   */
  bool hasAngleAlongFront() const;

  /**
   * Get the angle along the crack front from the beginning of the crack to the
   * specified position
   * @param point_index Index of the point
   * @return Angle along crack
   */
  Real getAngleAlongFront(const std::size_t point_index) const;

  /**
   * Get the number of points defining the crack front as a set of line segments
   * @return Number of points
   */
  std::size_t getNumCrackFrontPoints() const;

  /**
   * Whether the fracture computations are treated as 2D for the model
   * @return true if treated as 2D
   */
  bool treatAs2D() const { return _treat_as_2d; }

  /**
   * Is the crack defined by a mesh cutter object
   * @return true if using a mesh cutter
   */
  bool usingMeshCutter() const { return _use_mesh_cutter; }

  /**
   * Rotate a vector in the global coordinate coordinate system to the crack
   * front local coordinate system at a specified point on the crack.
   * @param vector Vector in global coordinates
   * @param point_index Index of the point
   * @return Vector in crack local coordinates
   */
  RealVectorValue rotateToCrackFrontCoords(const RealVectorValue vector,
                                           const std::size_t point_index) const;
  /**
   * Rotate a RankTwoTensor in the global coordinate coordinate system to the crack
   * front local coordinate system at a specified point on the crack.
   * @param tensor Tensor in global coordinates
   * @param point_index Index of the point
   * @return Tensor in crack local coordinates
   */
  RankTwoTensor rotateToCrackFrontCoords(const RankTwoTensor tensor,
                                         const std::size_t point_index) const;

  /**
   * Rotate a vector from crack front cartesian coordinate to global cartesian coordinate
   * @param vector Vector in crack local coordinates
   * @param point_index Index of the point
   * @return Vector in global coordinates
   */
  RealVectorValue rotateFromCrackFrontCoordsToGlobal(const RealVectorValue vector,
                                                     const std::size_t point_index) const;

  /**
   * Calculate r and theta of a point in the crack front polar coordinates for a given
   * crack point index.
   * @param qp The Point for which coordinates are evaluated
   * @param point_index the crack front point index
   * @param r Value of the radial coordinate computed in this function
   * @param theta Value of the theta coordinate computed in this function
   */
  void calculateRThetaToCrackFront(const Point qp,
                                   const std::size_t point_index,
                                   Real & r,
                                   Real & theta) const;

  /**
   * Calculate r and theta of a point in the crack front polar coordinate relative to the
   * closest crack front point. This function loops over all crack front points
   * to find the one closest to the specified point
   * @param qp The Point for which coordinates are evaluated
   * @param r Value of the radial coordinate computed in this function
   * @param theta Value of the theta coordinate computed in this function
   * @return Index of the closest crack front point
   */
  std::size_t calculateRThetaToCrackFront(const Point qp, Real & r, Real & theta) const;

  /**
   * Determine whether a given node is on one of the boundaries that intersects an end
   * of the crack front
   * @param node Pointer to node
   * @return true if the node is on an intersecting boundary
   */
  bool isNodeOnIntersectingBoundary(const Node * const node) const;

  /**
   * Determine whether a given crack front point is on one of the boundaries that
   * intersects an end of the crack front
   * @param point_index the crack front point index
   * @return true if the point is on an intersecting boundary
   */
  bool isPointWithIndexOnIntersectingBoundary(const std::size_t point_index) const;

  /**
   * Get the strain in the direction tangent to the crack front at a given point
   * @param node_index the crack front node index
   * @return Tangential strain
   */
  Real getCrackFrontTangentialStrain(const std::size_t node_index) const;

  /**
   * Determine whether the crack front was defined using nodes
   * @return true if it was defined using nodes
   */
  bool hasCrackFrontNodes() const
  {
    return _geom_definition_method == CRACK_GEOM_DEFINITION::CRACK_FRONT_NODES;
  }

  /**
   * Determine whether a node is contained within a specified volume integral element ring
   * for a given node on the crack front
   * @param ring_index Index of the ring
   * @param connected_node_id ID of the node
   * @param node_index Index of the crack front node
   * @return true if the node is in the ring
   */
  bool isNodeInRing(const std::size_t ring_index,
                    const dof_id_type connected_node_id,
                    const std::size_t node_index) const;

  /**
   * Compute the q function for the case where it is defined geometrically
   * @param crack_front_point_index Index of the point on the crack front
   * @param ring_index Index of the volume integral ring
   * @param current_node Node at which q is evaluated
   * @return q
   */
  Real DomainIntegralQFunction(std::size_t crack_front_point_index,
                               std::size_t ring_index,
                               const Node * const current_node) const;

  /**
   * Compute the q function for the case where it is defined through element connectivity
   * @param crack_front_point_index Index of the point on the crack front
   * @param ring_index Index of the volume integral ring
   * @param current_node Node at which q is evaluated
   * @return q
   */
  Real DomainIntegralTopologicalQFunction(std::size_t crack_front_point_index,
                                          std::size_t ring_index,
                                          const Node * const current_node) const;

  /**
    Set the value of _is_cutter_modified
   */
  void isCutterModified(const bool is_cutter_modified);

protected:
  /// Enum used to define the method for computing the crack extension direction
  const enum class DIRECTION_METHOD {
    CRACK_DIRECTION_VECTOR,
    CRACK_MOUTH,
    CURVED_CRACK_FRONT
  } _direction_method;

  /// Enum used to define the method for computing the crack extension direction
  /// at the ends of the crack
  const enum class END_DIRECTION_METHOD {
    NO_SPECIAL_TREATMENT,
    END_CRACK_DIRECTION_VECTOR,
    END_CRACK_TANGENT_VECTOR
  } _end_direction_method;

  /// Enum used to define the type of the nodes on the crack front (end or middle)
  enum CRACK_NODE_TYPE
  {
    MIDDLE_NODE,
    END_1_NODE,
    END_2_NODE
  };

  /// Enum used to define whether the crack front is defined using nodes or points
  enum class CRACK_GEOM_DEFINITION
  {
    CRACK_FRONT_NODES,
    CRACK_FRONT_POINTS
  } _geom_definition_method;

  /// Reference to the auxiliary system
  AuxiliarySystem & _aux;
  /// Reference to the mesh
  MooseMesh & _mesh;
  /// Tolerance used in geometric calculations
  static const Real _tol;

  /// Crack front nodes ordered from the start to end of the crack front
  std::vector<dof_id_type> _ordered_crack_front_nodes;
  /// Vector of points along the crack front
  std::vector<Point> _crack_front_points;
  /// Vector of tangent directions along the crack front
  std::vector<RealVectorValue> _tangent_directions;
  /// Vector of crack extension directions along the crack front
  std::vector<RealVectorValue> _crack_directions;
  /// Vector of segment lengths along the crack front
  std::vector<std::pair<Real, Real>> _segment_lengths;
  /// Vector of distances along the crack front
  std::vector<Real> _distances_along_front;
  /// Vector of angles along the crack front
  std::vector<Real> _angles_along_front;
  /// Vector of tangential strain along the crack front
  std::vector<Real> _strain_along_front;
  /// Vector of rotation matrices along the crack front
  std::vector<RankTwoTensor> _rot_matrix;
  /// Overall length of the crack
  Real _overall_length;
  /// Fixed vector optionally used to define crack extension direction
  RealVectorValue _crack_direction_vector;
  /// Fixed vector optionally used to define crack extension direction at end 1 of crack front
  RealVectorValue _crack_direction_vector_end_1;
  /// Fixed vector optionally used to define crack extension direction at end 2 of crack front
  RealVectorValue _crack_direction_vector_end_2;
  /// Fixed vector optionally used to define crack tangent direction at end 1 of crack front
  RealVectorValue _crack_tangent_vector_end_1;
  /// Fixed vector optionally used to define crack tangent direction at end 2 of crack front
  RealVectorValue _crack_tangent_vector_end_2;
  /// Names of boundaries used to define location of crack mouth
  std::vector<BoundaryName> _crack_mouth_boundary_names;
  /// IDs of boundaries used to define location of crack mouth
  std::vector<BoundaryID> _crack_mouth_boundary_ids;
  /// Names of boundaries that intersect crack at its ends
  std::vector<BoundaryName> _intersecting_boundary_names;
  /// IDs of boundaries that intersect crack at its ends
  std::vector<BoundaryID> _intersecting_boundary_ids;
  /// Coordinates of crack mouth
  RealVectorValue _crack_mouth_coordinates;
  /// Vector normal to the crack plane of a planar crack
  RealVectorValue _crack_plane_normal;
  /// Vector normals to a nonplanar crack described by the cutter mesh when _use_mesh_cutter = true
  std::vector<RealVectorValue> _crack_plane_normals;
  /// Whether to treat the model as 2D for computation of fracture integrals
  bool _treat_as_2d;
  /// Whether to describe the crack as a mesh cutter
  bool _use_mesh_cutter;
  /// Indicator that shows if the cutter mesh is modified or not in the calculation step
  bool _is_cutter_modified;
  /// Whether the crack forms a closed loop
  bool _closed_loop;
  /// Out of plane axis when crack is treated as 2D
  unsigned int _axis_2d;
  /// Whether the crack plane is also a symmetry plane in the model
  bool _has_symmetry_plane;
  /// Which plane is the symmetry plane
  unsigned int _symmetry_plane;
  /// Names of the x, y, and z displacement variables
  ///@{
  std::string _disp_x_var_name;
  std::string _disp_y_var_name;
  std::string _disp_z_var_name;
  ///@}
  /// Whether the T-stress is being computed
  bool _t_stress;
  /// Whether topological rings are used to define the q functions
  bool _q_function_rings;
  /// Numer of elements from crack tip to last topological ring
  std::size_t _last_ring;
  /// Numer of elements from crack tip to first topological ring
  std::size_t _first_ring;
  /// Data structure used to store information about topological rings
  /// Key is a pair of the crack front node index and ring id
  /// Data is a set of the IDs of the nodes in the ring for that crack front node
  std::map<std::pair<dof_id_type, std::size_t>, std::set<dof_id_type>>
      _crack_front_node_to_node_map;
  /// Method used to define the q function
  MooseEnum _q_function_type;
  /// Vector of bools indicating whether individual crack front points are on
  /// an intersecting boundary
  std::vector<bool> _is_point_on_intersecting_boundary;
  /// Vector of inner radii of the rings used for geometric q functions
  std::vector<Real> _j_integral_radius_inner;
  /// Vector of outer radii of the rings used for geometric q functions
  std::vector<Real> _j_integral_radius_outer;
  /// Pointer to a CrackFrontPointsProvider object optionally used to define
  /// the crack front points
  const CrackFrontPointsProvider * _crack_front_points_provider;
  /// Number of points coming from the CrackFrontPointsProvider
  std::size_t _num_points_from_provider;

  /**
   * Get the set of all crack front nodes
   * @param nodes Set of nodes -- populated by this method
   */
  void getCrackFrontNodes(std::set<dof_id_type> & nodes);

  /**
   * Arrange the crack front nodes by their position along the crack front,
   * and put them in the _ordered_crack_front_nodes member variable.
   * @param nodes Set of nodes to be ordered
   */
  void orderCrackFrontNodes(std::set<dof_id_type> & nodes);

  /**
   * Determine which of the end nodes should be the starting point of the
   * crack front.
   * @param end_nodes Vector containing two end nodes. The order of this is
   *                  rearranged so that the first end node is the start of
   *                  the crack front, and the second is at the end.
   */
  void orderEndNodes(std::vector<dof_id_type> & end_nodes);

  /**
   * For the case of a crack that is a complete loop, determine which of the
   * nodes should be the start and end nodes in a repeatable way.
   * @param end_nodes Vector containing two end nodes -- populated by this method
   * @param nodes Set of all nodes on the crack front
   * @param node_to_line_elem_map Map from crack front nodes to line elements on crack
   *                              front (see line_elems param)
   * @param line_elems Line elements on crack front defined as vectors of the
   *                   nodes on each end of the line elements.
   */
  void
  pickLoopCrackEndNodes(std::vector<dof_id_type> & end_nodes,
                        std::set<dof_id_type> & nodes,
                        std::map<dof_id_type, std::vector<dof_id_type>> & node_to_line_elem_map,
                        std::vector<std::vector<dof_id_type>> & line_elems);
  /**
   * Find the node with the maximum value of its coordinate. This is used to
   * deterministically find the first node on the crack front. First, the nodes
   * with the maximum coordinate in one direction are found, and then if there
   * are duplicates with that same coordinate, search through the other two
   * coordinates to find the node with the maximum coordinate in all 3 directions.
   * @param nodes Set of all nodes on the crack front
   * @param dir0 First coordinate direction in which to order the coordinates
   */
  dof_id_type maxNodeCoor(std::vector<Node *> & nodes, unsigned int dir0 = 0);

  /**
   * Update the data structures defining the crack front geometry such as
   * the ordered crack front nodes/points and other auxiliary data.
   */
  void updateCrackFrontGeometry();

  /**
   * Update the data structures used to determine the crack front direction
   * vectors such as crack mouth coordinates.
   */
  void updateDataForCrackDirection();

  /**
   * Compute the direction of crack extension for a given point on the crack front.
   * @param crack_front_point Point on the crack front
   * @param tangent_direction Tangent direction vector for the crack front point
   * @param ntype Node type such as MIDDLE_NODE, END_1_NODE, END_2_NODE
   * @param crack_front_point_index Index of the point on the crack front
   */
  RealVectorValue calculateCrackFrontDirection(const Point & crack_front_point,
                                               const RealVectorValue & tangent_direction,
                                               const CRACK_NODE_TYPE ntype,
                                               const std::size_t crack_front_point_index = 0) const;

  /**
   * Compute the strain in the direction tangent to the crack at all points on the
   * crack front.
   */
  void calculateTangentialStrainAlongFront();

  /**
   * Create the data defining the rings used to define the q function when the topological
   * option is used to define the q function.
   */
  void createQFunctionRings();

  /**
   * Find nodes that are connected through elements to the nodes in the previous
   * node ring.
   * @param nodes_new_ring Nodes in the new ring -- populated by this method
   * @param nodes_old_ring Nodes in the previous ring
   * @param nodes_all_rings Nodes in all other rings to be excluded from the new ring
   * @param nodes_neighbor1 Nodes in the neighboring ring to one side to be excluded from the new
   * ring
   * @param nodes_neighbor2 Nodes in the neighboring ring to the other side to be excluded from the
   * new ring
   * @param nodes_to_elem_map Map of nodes to connected elements
   */
  void addNodesToQFunctionRing(std::set<dof_id_type> & nodes_new_ring,
                               const std::set<dof_id_type> & nodes_old_ring,
                               const std::set<dof_id_type> & nodes_all_rings,
                               const std::set<dof_id_type> & nodes_neighbor1,
                               const std::set<dof_id_type> & nodes_neighbor2,
                               std::vector<std::vector<const Elem *>> & nodes_to_elem_map);

  /**
   * Project a point to a specified point along the crack front and compute the
   * projected normal and tangential distance to the front
   * @param dist_to_front Projected normal distance to the front -- computed by this method
   * @param dist_along_tangent Project tangent distance to the front -- computed by this method
   * @param crack_front_point_index Index of the point on the crack front
   * @param current_node Node to be projected to the front
   */
  void projectToFrontAtPoint(Real & dist_to_front,
                             Real & dist_along_tangent,
                             std::size_t crack_front_point_index,
                             const Node * const current_node) const;
};
