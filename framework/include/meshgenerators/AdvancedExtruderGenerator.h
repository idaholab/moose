//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"

#include "libmesh/point.h"

/**
 * Extrudes a mesh to another dimension
 */
class AdvancedExtruderGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  AdvancedExtruderGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Mesh that comes from another generator
  std::unique_ptr<MeshBase> & _input;

  /// Height of each elevation
  const std::vector<Real> & _heights;

  /// Bias growth factor of each elevation
  const std::vector<Real> _biases;

  /// Number of layers in each elevation
  const std::vector<unsigned int> & _num_layers;

  /// Subdomains to swap out for each elevation
  const std::vector<std::vector<subdomain_id_type>> & _subdomain_swaps;

  /// Boundaries to swap out for each elevation
  const std::vector<std::vector<boundary_id_type>> & _boundary_swaps;

  /// Names and indices of extra element integers to swap
  const std::vector<std::string> & _elem_integer_names_to_swap;
  std::vector<unsigned int> _elem_integer_indices_to_swap;

  /// Extra element integers to swap out for each elevation and each element interger name
  const std::vector<std::vector<std::vector<dof_id_type>>> & _elem_integers_swaps;

  /// Easier to work with version of _sudomain_swaps
  std::vector<std::unordered_map<subdomain_id_type, subdomain_id_type>> _subdomain_swap_pairs;

  /// Easier to work with version of _boundary_swaps
  std::vector<std::unordered_map<boundary_id_type, boundary_id_type>> _boundary_swap_pairs;

  /// Easier to work with version of _elem_integers_swaps
  std::vector<std::unordered_map<dof_id_type, dof_id_type>> _elem_integers_swap_pairs;

  /// The direction of the extrusion
  Point _direction;

  // Attributes for extruding along a curve mesh of edges
  /// Curve mesh to extrude along
  std::unique_ptr<MeshBase> & _extrusion_curve;
  /// Extrusion direction to follow at the start (first layer) of the extrusion
  const RealVectorValue _start_extrusion_direction;
  /// Extrusion direction for the final layer of the extrusion
  const RealVectorValue _end_extrusion_direction;
  /// Whether we are extruding along a curve
  bool _extrude_along_curve;

  /// Whether to extrude each node along the averaged normal of the elements connected to it
  const bool _extrude_along_node_normals;

  const bool _has_top_boundary;
  const BoundaryName _top_boundary;

  const bool _has_bottom_boundary;
  const BoundaryName _bottom_boundary;

  /// The list of input mesh's blocks that need to be assigned upward boundary interfaces for each layer of elevation
  const std::vector<std::vector<subdomain_id_type>> _upward_boundary_source_blocks;

  /// Upward boundary interfaces for each layer of elevation
  const std::vector<std::vector<boundary_id_type>> _upward_boundary_ids;

  /// The list of input mesh's blocks that need to be assigned downward boundary interfaces for each layer of elevation
  const std::vector<std::vector<subdomain_id_type>> _downward_boundary_source_blocks;

  /// Downward boundary interfaces for each layer of elevation
  const std::vector<std::vector<boundary_id_type>> _downward_boundary_ids;

  /// Axial pitch for a full rotation
  const Real _twist_pitch;

  // Radial transformation attributes
  /// Radial extent of the extruded geometry at the end  of the extrusion
  const Real _end_radial_extent;
  /// Function type for modifying the radial extent of the extruded shape
  const MooseEnum _radial_expansion_method;
  /// Derivative of the radial expansion function at the beginning of the extrusion
  const Real _start_radial_growth_rate;
  /// Derivative of the radial expansion function at the end of the extrusion
  const Real _end_radial_growth_rate;

  /// Calculate the share of the radial expansion to apply at the local node
  /// This share goes from 0 at the beginning of the extrusion to 1 at the end
  /// @param t coordinate along the curve (in the axial direction of extrusion)
  Real radialExpansionRatio(const Real t) const;

  /// Compute, for each node, the extrusion direction as the average of the normals of all the
  /// elements connected to it. When connected elements disagree on the surface orientation, the
  /// minority normals are flipped to match the majority before averaging.
  /// @param input the 2D mesh being extruded
  /// @return a map from node id to its (unit) extrusion direction
  std::unordered_map<dof_id_type, Point> computeNodeNormals(const MeshBase & input) const;
};
