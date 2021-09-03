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
#include "libmesh/mesh_smoother_laplace.h"

class PolygonConcentricCircleMeshGeneratorBase : public PolygonMeshGeneratorBase
{
public:
  static InputParameters validParams();

  PolygonConcentricCircleMeshGeneratorBase(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  const unsigned int _num_sides;
  // Radii of concentric circles
  std::vector<Real> _ring_radii;

  /// Number of rings in each circle or in the enclosing square
  const std::vector<unsigned int> _ring_intervals;
  const std::vector<subdomain_id_type> _ring_block_ids;
  const std::vector<SubdomainName> _ring_block_names;

  // Thickness of each enclosing duct
  const enum class DuctStyle { apothem, radius } _duct_sizes_style;

  std::vector<Real> _duct_sizes;

  /// Number of layers in each enclosing duct
  const std::vector<unsigned int> _duct_intervals;
  const std::vector<subdomain_id_type> _duct_block_ids;
  const std::vector<SubdomainName> _duct_block_names;
  const bool _has_rings;
  const bool _has_ducts;
  const enum class PolygonStyle { apothem, radius } _polygon_size_style;
  const Real _polygon_size;
  const std::vector<unsigned int> _num_sectors_per_side;
  const unsigned int _background_intervals;
  const std::vector<subdomain_id_type> _background_block_ids;
  const std::vector<SubdomainName> _background_block_names;

  /// Volume preserving function is optional
  const bool _preserve_volumes;

  // const bool _uniform_mesh_on_sides;
  const unsigned int _block_id_shift;
  const unsigned int _interface_boundary_id_shift;

  // const unsigned int _smoothing_max_it;
  const boundary_id_type _external_boundary_id;
  const std::string _external_boundary_name;
  const std::vector<std::string> _interface_boundary_names;

  const std::vector<unsigned int> _sides_to_adapt;
  std::vector<std::unique_ptr<MeshBase> *> _input_ptrs;

  bool _uniform_mesh_on_sides;
  bool _quad_center_elements;
  unsigned int _smoothing_max_it;
  bool _is_control_drum_meta;
  bool _is_general_polygon;

  Real & _pitch_meta;
  unsigned int & _background_intervals_meta;
  dof_id_type & _node_id_background_meta;
  Real _pitch;
  std::vector<unsigned int> _num_sectors_per_side_meta;
  std::vector<std::vector<Real>> azimuthal_angles_array;
  Real & _pattern_pitch_meta;
  std::vector<Real> & _azimuthal_angle_meta;
};
