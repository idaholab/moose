//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "PolygonMeshGeneratorBase.h"

/**
 * This RevolveGenerator object is designed to revolve a 1D mesh into 2D, or a 2D mesh into 3D based
 * on an axis.
 */
class RevolveGenerator : public PolygonMeshGeneratorBase
{
public:
  static InputParameters validParams();

  RevolveGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Lower dimensional mesh from another generator
  std::unique_ptr<MeshBase> & _input;

  /// A point of the axis of revolution
  const Point & _axis_point;

  /// A direction vector of the axis of revolution
  const Point & _axis_direction;

  /// Angles of revolution delineating each azimuthal section
  const std::vector<Real> _revolving_angles;

  /// Subdomains to swap out for each azimuthal section
  const std::vector<std::vector<subdomain_id_type>> & _subdomain_swaps;

  /// Boundaries to swap out for each elevation
  const std::vector<std::vector<boundary_id_type>> & _boundary_swaps;

  /// Names and indices of extra element integers to swap
  const std::vector<std::string> & _elem_integer_names_to_swap;
  std::vector<unsigned int> _elem_integer_indices_to_swap;

  /// Extra element integers to swap out for each elevation and each element integer name
  const std::vector<std::vector<std::vector<dof_id_type>>> & _elem_integers_swaps;

  /// Revolving direction
  const bool & _clockwise;

  /// Numbers of azimuthal mesh intervals in each azimuthal section
  const std::vector<unsigned int> & _nums_azimuthal_intervals;

  /// Volume preserving function is optional
  const bool _preserve_volumes;

  /// Whether a starting boundary is specified
  bool _has_start_boundary;

  /// Boundary ID of the starting boundary
  boundary_id_type _start_boundary;

  /// Whether an ending boundary is specified
  bool _has_end_boundary;

  /// Easier to work with version of _sudomain_swaps
  std::vector<std::unordered_map<subdomain_id_type, subdomain_id_type>> _subdomain_swap_pairs;

  /// Easier to work with version of _boundary_swaps
  std::vector<std::unordered_map<boundary_id_type, boundary_id_type>> _boundary_swap_pairs;

  /// Easier to work with version of _elem_integers_swaps
  std::vector<std::unordered_map<dof_id_type, dof_id_type>> _elem_integers_swap_pairs;

  /// Whether to revolve for a full circle or not
  bool _full_circle_revolving;

  /// Unit angles of all azimuthal sections of revolution
  std::vector<Real> _unit_angles;

  /// Boundary ID of the ending boundary
  boundary_id_type _end_boundary;

  /// Radius correction factor
  Real _radius_correction_factor;

  /**
   * Get the rotation center and radius of the circular rotation based on the rotation axis and the
   * external point.
   * @param p_ext external point that needs to be rotated
   * @param p_axis a point on the rotation axis
   * @param dir_axis direction vector of the rotation axis
   * @return a pair of the rotation center and the radius of the circular rotation
   */
  std::pair<Real, Point> getRotationCenterAndRadius(const Point & p_ext,
                                                    const Point & p_axis,
                                                    const Point & dir_axis) const;

  /**
   * Calculate the transform matrix between the rotation coordinate system and the original
   * coordinate system.
   * @param p_axis a point on the rotation axis
   * @param dir_axis direction vector of the rotation axis
   * @param p_input a point in the input mesh
   * @return a transform matrix, stored as 3 points in a vector (each point represents a row of the
   * matrix)
   */
  std::vector<Point>
  rotationVectors(const Point & p_axis, const Point & dir_axis, const Point & p_input) const;

  /**
   * Categorize the nodes of an element into two groups: nodes on the axis and nodes off the axis.
   * @param elem the element whose nodes are to be categorized
   * @param nodes_on_axis a list of node IDs on the axis
   * @return a pair of lists of node IDs: the first list is for nodes on the axis, and the
   * second list is for nodes off the axis
   */
  std::pair<std::vector<dof_id_type>, std::vector<dof_id_type>>
  onAxisNodesIdentifier(const Elem & elem, const std::vector<dof_id_type> & nodes_on_axis) const;

  /**
   * Modify the position of a node to account for radius correction.
   * @param node the node to be modified
   */
  void nodeModification(Node & node);

  /**
   * Create a new QUAD element from an existing EDGE element by revolving it.
   * @param quad_elem_type the type of the new QUAD element
   * @param elem the EDGE element to be revolved
   * @param mesh the mesh that the new QUAD element will be added to
   * @param new_elem the new QUAD element
   * @param current_layer the current azimuthal layer
   * @param orig_nodes the number of nodes in the original mesh
   * @param total_num_azimuthal_intervals the total number of azimuthal intervals for revolving
   * @param side_pairs a vector of pairs to record the corresponding side indices of the original
   * and the new elements
   * @param is_flipped a flag to indicate whether the new element is flipped after creation (to
   * ensure a positive element volume)
   */
  void createQUADfromEDGE(const ElemType quad_elem_type,
                          const Elem * elem,
                          const std::unique_ptr<MeshBase> & mesh,
                          std::unique_ptr<Elem> & new_elem,
                          const int current_layer,
                          const unsigned int orig_nodes,
                          const unsigned int total_num_azimuthal_intervals,
                          std::vector<std::pair<dof_id_type, dof_id_type>> & side_pairs,
                          bool & is_flipped) const;

  /**
   * Create a new TRI element from an existing EDGE element by revolving it.
   * @param nodes_cates a pair of two lists of node IDs: the first list is for nodes on the axis,
   * and the second list is for nodes off the axis
   * @param tri_elem_type the type of the new TRI element
   * @param elem the EDGE element to be revolved
   * @param mesh the mesh that the new TRI element will be added to
   * @param new_elem the new TRI element
   * @param current_layer the current layer of the revolving
   * @param orig_nodes the number of nodes in the original mesh
   * @param total_num_azimuthal_intervals the total number of azimuthal intervals for revolving
   * @param side_pairs a vector of pairs to record the corresponding side indices of the original
   * and the new elements
   * @param axis_node_case  a parameter to record on-axis node(s)
   * @param is_flipped a flag to indicate whether the new element is flipped after creation (to
   * ensure a positive element volume)
   */
  void createTRIfromEDGE(
      const std::pair<std::vector<dof_id_type>, std::vector<dof_id_type>> & nodes_cates,
      const ElemType tri_elem_type,
      const Elem * elem,
      const std::unique_ptr<MeshBase> & mesh,
      std::unique_ptr<Elem> & new_elem,
      const int current_layer,
      const unsigned int orig_nodes,
      const unsigned int total_num_azimuthal_intervals,
      std::vector<std::pair<dof_id_type, dof_id_type>> & side_pairs,
      dof_id_type & axis_node_case,
      bool & is_flipped) const;

  /**
   * Create a new PRISM element from an existing TRI element by revolving it.
   * @param prism_elem_type the type of the new TET element
   * @param elem the TRI element to be revolved
   * @param mesh the mesh that the new TET element will be added to
   * @param new_elem the new TET element
   * @param current_layer the current layer of the revolving
   * @param orig_nodes the number of nodes in the original mesh
   * @param total_num_azimuthal_intervals the total number of azimuthal intervals for revolving
   * @param side_pairs a vector of pairs to record the corresponding side indices of the original
   * and the new elements
   * @param is_flipped a flag to indicate whether the new element is flipped after creation (to
   * ensure a positive element volume)
   */
  void createPRISMfromTRI(const ElemType prism_elem_type,
                          const Elem * elem,
                          const std::unique_ptr<MeshBase> & mesh,
                          std::unique_ptr<Elem> & new_elem,
                          const int current_layer,
                          const unsigned int orig_nodes,
                          const unsigned int total_num_azimuthal_intervals,
                          std::vector<std::pair<dof_id_type, dof_id_type>> & side_pairs,
                          bool & is_flipped) const;

  /**
   * Create a new PYRAMID element from an existing TRI element by revolving it.
   * @param nodes_cates a pair of two lists of node IDs: the first list is for nodes on the axis,
   * and the second list is for nodes off the axis
   * @param pyramid_elem_type the type of the new PYRAMID element
   * @param elem the TRI element to be revolved
   * @param mesh the mesh that the new PYRAMID element will be added to
   * @param new_elem the new PYRAMID element
   * @param current_layer the current layer of the revolving
   * @param orig_nodes the number of nodes in the original mesh
   * @param total_num_azimuthal_intervals the total number of azimuthal intervals for revolving
   * @param side_pairs a vector of pairs to record the corresponding side indices of the original
   * and the new elements
   * @param axis_node_case a parameter to record on-axis node(s)
   * @param is_flipped a flag to indicate whether the new element is flipped after creation (to
   * ensure a positive element volume)
   */
  void createPYRAMIDfromTRI(
      const std::pair<std::vector<dof_id_type>, std::vector<dof_id_type>> & nodes_cates,
      const ElemType pyramid_elem_type,
      const Elem * elem,
      const std::unique_ptr<MeshBase> & mesh,
      std::unique_ptr<Elem> & new_elem,
      const int current_layer,
      const unsigned int orig_nodes,
      const unsigned int total_num_azimuthal_intervals,
      std::vector<std::pair<dof_id_type, dof_id_type>> & side_pairs,
      dof_id_type & axis_node_case,
      bool & is_flipped) const;

  /**
   * Create a new TET element from an existing TRI element by revolving it.
   * @param nodes_cates a pair of two lists of node IDs: the first list is for nodes on the axis,
   * and the second list is for nodes off the axis
   * @param tet_elem_type the type of the new TET element
   * @param elem the TRI element to be revolved
   * @param mesh the mesh that the new TET element will be added to
   * @param new_elem the new TET element
   * @param current_layer the current layer of the revolving
   * @param orig_nodes the number of nodes in the original mesh
   * @param total_num_azimuthal_intervals the total number of azimuthal intervals for revolving
   * @param side_pairs a vector of pairs to record the corresponding side indices of the original
   * and the new elements
   * @param axis_node_case a parameter to record on-axis node(s)
   * @param is_flipped a flag to indicate whether the new element is flipped after creation (to
   * ensure a positive element volume)
   */
  void createTETfromTRI(
      const std::pair<std::vector<dof_id_type>, std::vector<dof_id_type>> & nodes_cates,
      const ElemType tet_elem_type,
      const Elem * elem,
      const std::unique_ptr<MeshBase> & mesh,
      std::unique_ptr<Elem> & new_elem,
      const int current_layer,
      const unsigned int orig_nodes,
      const unsigned int total_num_azimuthal_intervals,
      std::vector<std::pair<dof_id_type, dof_id_type>> & side_pairs,
      dof_id_type & axis_node_case,
      bool & is_flipped) const;

  /**
   * Create a new HEX element from an existing QUAD element by revolving it.
   * @param hex_elem_type the type of the new HEX element
   * @param elem the QUAD element to be revolved
   * @param mesh the mesh that the new HEX element will be added to
   * @param new_elem the new HEX element
   * @param current_layer the current layer of the revolving
   * @param orig_nodes the number of nodes in the original mesh
   * @param total_num_azimuthal_intervals the total number of azimuthal intervals for revolving
   * @param side_pairs a vector of pairs to record the corresponding side indices of the original
   * and the new elements
   * @param is_flipped a flag to indicate whether the new element is flipped after creation (to
   * ensure a positive element volume)
   */
  void createHEXfromQUAD(const ElemType hex_elem_type,
                         const Elem * elem,
                         const std::unique_ptr<MeshBase> & mesh,
                         std::unique_ptr<Elem> & new_elem,
                         const int current_layer,
                         const unsigned int orig_nodes,
                         const unsigned int total_num_azimuthal_intervals,
                         std::vector<std::pair<dof_id_type, dof_id_type>> & side_pairs,
                         bool & is_flipped) const;

  /**
   * Create a new PRISM element from an existing QUAD element by revolving it.
   * @param nodes_cates a pair of two lists of node IDs: the first list is for nodes on the axis,
   * and the second list is for nodes off the axis
   * @param prism_elem_type the type of the new PRISM element
   * @param elem the QUAD element to be revolved
   * @param mesh the mesh that the new PRISM element will be added to
   * @param new_elem the new PRISM element
   * @param current_layer the current layer of the revolving
   * @param orig_nodes the number of nodes in the original mesh
   * @param total_num_azimuthal_intervals the total number of azimuthal intervals for revolving
   * @param side_pairs a vector of pairs to record the corresponding side indices of the original
   * and the new elements
   * @param axis_node_case a parameter to record on-axis node(s)
   * @param is_flipped a flag to indicate whether the new element is flipped after creation (to
   * ensure a positive element volume)
   */
  void createPRISMfromQUAD(
      const std::pair<std::vector<dof_id_type>, std::vector<dof_id_type>> & nodes_cates,
      const ElemType prism_elem_type,
      const Elem * elem,
      const std::unique_ptr<MeshBase> & mesh,
      std::unique_ptr<Elem> & new_elem,
      const int current_layer,
      const unsigned int orig_nodes,
      const unsigned int total_num_azimuthal_intervals,
      std::vector<std::pair<dof_id_type, dof_id_type>> & side_pairs,
      dof_id_type & axis_node_case,
      bool & is_flipped) const;

  /**
   * Create a new PYRAMID element and a new PRISM element from an existing QUAD element by revolving
   * it.
   * @param nodes_cates a pair of two lists of node IDs: the first list is for nodes on the axis,
   * and the second list is for nodes off the axis
   * @param pyramid_elem_type the type of the new PYRAMID element
   * @param prism_elem_type the type of the new PRISM element
   * @param elem the QUAD element to be revolved
   * @param mesh the mesh that the new PYRAMID element will be added to
   * @param new_elem the new PYRAMID element
   * @param new_elem_1 the new PRISM element
   * @param current_layer the current layer of the revolving
   * @param orig_nodes the number of nodes in the original mesh
   * @param total_num_azimuthal_intervals the total number of azimuthal intervals for revolving
   * @param side_pairs a vector of pairs to record the corresponding side indices of the original
   * and the new elements
   * @param axis_node_case a parameter to record on-axis node(s)
   * @param is_flipped a flag to indicate whether the PYRAMID element is flipped after creation (to
   * ensure a positive element volume)
   * @param is_flipped_additional a flag to indicate whether the PRISM element is flipped after (to
   * ensure a positive element volume) creation
   */
  void createPYRAMIDPRISMfromQUAD(
      const std::pair<std::vector<dof_id_type>, std::vector<dof_id_type>> & nodes_cates,
      const ElemType pyramid_elem_type,
      const ElemType prism_elem_type,
      const Elem * elem,
      const std::unique_ptr<MeshBase> & mesh,
      std::unique_ptr<Elem> & new_elem,
      std::unique_ptr<Elem> & new_elem_1,
      const int current_layer,
      const unsigned int orig_nodes,
      const unsigned int total_num_azimuthal_intervals,
      std::vector<std::pair<dof_id_type, dof_id_type>> & side_pairs,
      dof_id_type & axis_node_case,
      bool & is_flipped,
      bool & is_flipped_additional) const;
};
