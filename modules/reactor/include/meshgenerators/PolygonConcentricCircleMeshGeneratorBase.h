//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ConcentricCircleGeneratorBase.h"

/**
 * This PolygonConcentricCircleMeshGeneratorBase object is a base class to be inherited for polygon
 * mesh generators.
 */
class PolygonConcentricCircleMeshGeneratorBase : public ConcentricCircleGeneratorBase
{
public:
  static InputParameters validParams();

  PolygonConcentricCircleMeshGeneratorBase(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Number of polygon sides
  const unsigned int _num_sides;
  /// Thickness of each enclosing duct
  const PolygonSizeStyle _duct_sizes_style;
  /// Size parameters of the duct regions
  std::vector<Real> _duct_sizes;
  /// Number of layers in each enclosing duct
  const std::vector<unsigned int> _duct_intervals;
  /// Bias values used to induce biasing to radial meshing in duct regions
  const std::vector<Real> _duct_radial_biases;
  /// Widths, fractions, radial sectors and growth factors of the inner boundary layers of the duct regions
  multiBdryLayerParams _duct_inner_boundary_layer_params;
  /// Widths, fractions, radial sectors and growth factors of the inner boundary layers of the duct regions
  multiBdryLayerParams _duct_outer_boundary_layer_params;
  /// Subdomain IDs of the duct regions
  const std::vector<subdomain_id_type> _duct_block_ids;
  /// Subdomain Names of the duct regions
  const std::vector<SubdomainName> _duct_block_names;
  /// Whether the generated mesh contains ring regions
  const bool _has_rings;
  /// Whether the generated mesh contains duct regions
  const bool _has_ducts;
  /// Type of polygon size parameter
  const PolygonSizeStyle _polygon_size_style;
  /// Polygon size parameter
  const Real _polygon_size;
  /// Mesh sector number of each polygon side
  const std::vector<unsigned int> _num_sectors_per_side;
  /// Numbers of radial intervals of the background regions
  const unsigned int _background_intervals;
  /// Bias value used to induce biasing to radial meshing in background region
  const Real _background_radial_bias;
  /// Width, fraction, radiation sectors and growth factor of the inner boundary layer of the background region
  singleBdryLayerParams _background_inner_boundary_layer_params;
  /// Width, fraction, radiation sectors and growth factor of the outer boundary layer of the background region
  singleBdryLayerParams _background_outer_boundary_layer_params;
  /// Subdomain IDs of the background regions
  std::vector<subdomain_id_type> _background_block_ids;
  /// Subdomain Names of the background regions
  std::vector<SubdomainName> _background_block_names;
  /// Whether the nodes on the external boundary needs to be uniformly distributed
  const bool _uniform_mesh_on_sides;
  /// Whether the central elements need to be QUAD4
  const bool _quad_center_elements;
  /// A fractional radius factor used to determine the radial positions of transition nodes in the center region meshed by quad elements (default is 1.0 - 1.0/div_num)
  const Real _center_quad_factor;
  /// Maximum smooth iteration number
  const unsigned int _smoothing_max_it;
  /// Indices of the hexagon sides that need to adapt
  const std::vector<unsigned int> _sides_to_adapt;
  /// Pointers to input mesh pointers
  std::vector<std::unique_ptr<MeshBase> *> _input_ptrs;
  /// MeshMetaData: whether this produced mesh is a general polygon (or a hexagon)
  bool _is_general_polygon;
  /// MeshMetaData: maximum node id of the background region
  dof_id_type & _node_id_background_meta;
  /// MeshMetaData: whether this produced mesh is a control drum
  bool & _is_control_drum_meta;
  /// Pitch size of the produced polygon
  Real _pitch;
  /// Azimuthal angles of all radial nodes for volume preservation
  std::vector<std::vector<Real>> _azimuthal_angles_array;
};
