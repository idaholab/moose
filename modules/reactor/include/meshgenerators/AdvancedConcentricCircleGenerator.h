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
 * This AdvancedConcentricCircleGenerator object is designed to mesh a concentric circular geometry.
 */
class AdvancedConcentricCircleGenerator : public PolygonMeshGeneratorBase
{
public:
  static InputParameters validParams();

  AdvancedConcentricCircleGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// List of the azimuthal angles of the nodes
  std::vector<Real> _azimuthal_angles;
  /// Number of azimuthal sectors of the circular mesh to be generated
  const unsigned int _num_sectors;
  /// A virtual sector number list which are 360.0 times inverse of the azimuthal intervals
  std::vector<Real> _virtual_nums_sectors;
  /// Radii of concentric circles
  const std::vector<Real> _ring_radii;
  /// Number of rings in each circle or in the enclosing square
  const std::vector<unsigned int> _ring_intervals;
  /// Bias values used to induce biasing to radial meshing in ring regions
  const std::vector<Real> _ring_radial_biases;
  /// Widths, fractions, radial sectors and growth factors of the inner boundary layers of the ring regions
  multiBdryLayerParams _ring_inner_boundary_layer_params;
  /// Widths, fractions, radial sectors and growth factors of the outer boundary layers of the ring regions
  multiBdryLayerParams _ring_outer_boundary_layer_params;
  /// Subdomain IDs of the ring regions
  std::vector<subdomain_id_type> _ring_block_ids;
  /// Subdomain Names of the ting regions
  std::vector<SubdomainName> _ring_block_names;
  /// Volume preserving function is optional
  const bool _preserve_volumes;
  /// Shift in default subdomain IDs to avert potential conflicts
  const subdomain_id_type _block_id_shift;
  /// Whether inward interface boundaries are created
  const bool _create_inward_interface_boundaries;
  /// Whether outward interface boundaries are created
  const bool _create_outward_interface_boundaries;
  /// Shift in default boundary IDs of interfaces to avert potential conflicts
  const boundary_id_type _interface_boundary_id_shift;
  /// Boundary ID of the mesh's external boundary
  const boundary_id_type _external_boundary_id;
  /// Boundary Name of the mesh's external boundary
  const std::string _external_boundary_name;
  /// Boundary Names of the mesh's inward interface boundaries
  const std::vector<std::string> _inward_interface_boundary_names;
  /// Boundary Names of the mesh's outward interface boundaries
  const std::vector<std::string> _outward_interface_boundary_names;
  /// Whether side specific boundaries are created
  const bool _create_side_specific_boundaries;
};
