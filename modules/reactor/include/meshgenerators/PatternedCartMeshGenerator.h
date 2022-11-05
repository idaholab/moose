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
#include "MooseEnum.h"
#include "MeshMetaDataInterface.h"

/**
 * This PatternedCartMeshGenerator source code assembles square meshes into a rectangular grid and
 * optionally adds a duct.
 */
class PatternedCartMeshGenerator : public PolygonMeshGeneratorBase
{
public:
  static InputParameters validParams();

  PatternedCartMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// The input meshes
  const std::vector<std::unique_ptr<MeshBase> *> _mesh_ptrs;
  /// Names of input meshes
  const std::vector<MeshGeneratorName> & _input_names;
  /// 2D vector of the square pattern
  const std::vector<std::vector<unsigned int>> & _pattern;
  /// Type of the external boundary shape
  const MooseEnum _pattern_boundary;
  /// Whether a reactor core mesh with core metadata is generated
  const bool _generate_core_metadata;
  /// Number of radial intervals in the background region
  const unsigned int _background_intervals;
  /// Whether the square pattern has external duct(s)
  const bool _has_assembly_duct;
  /// Size parameter(s) of duct(s)
  std::vector<Real> _duct_sizes;
  /// Style of the duct size parameter(s)
  const PolygonSizeStyle _duct_sizes_style;
  /// Number(s) of radial intervals of duct layer(s)
  const std::vector<unsigned int> _duct_intervals;
  /// Whether the nodes on the external boundary are uniformly distributed
  const bool _uniform_mesh_on_sides;
  /// Whether a text file containing control drum positions is generated
  const bool _generate_control_drum_positions_file;
  /// Wheter control drum IDs are assigned as an extra element integer
  const bool _assign_control_drum_id;
  /// The mesh rotation angle after mesh generation
  const Real _rotate_angle;
  /// Subdomain IDs of the duct layers
  const std::vector<subdomain_id_type> _duct_block_ids;
  /// Subdomain Names of the duct layers
  const std::vector<SubdomainName> _duct_block_names;
  /// Boundary ID of mesh's external boundary
  const boundary_id_type _external_boundary_id;
  /// Boundary name of mesh's external boundary
  const std::string _external_boundary_name;
  /// Whether interface boundaries are created
  const bool _create_interface_boundaries;
  /// Style of the polygon size parameter
  const PolygonSizeStyle _square_size_style;
  /// Whether the non-circular region (outside the rings) can be deformed
  const bool _deform_non_circular_region;
  /// Pitch size of the input assembly mesh
  Real _pattern_pitch;
  /// MeshMetaData of the assembly pitch size
  Real & _pattern_pitch_meta;
  /// MeshMetaData of input square's pitch
  Real & _input_pitch_meta;
  /// MeshMetaData: whether the generated mesh is a control drum
  const bool _is_control_drum_meta;
  /// MeshMetaData: positions of the control drums within the generated core mesh
  std::vector<Point> & _control_drum_positions;
  /// MeshMetaData: azimuthal angles of the control drum centers within the generated core mesh
  std::vector<Real> & _control_drum_angles;
  /// MetaMeshData: azimuthal angles of all the nodes of each control drum within the generated core mesh
  std::vector<std::vector<Real>> & _control_drums_azimuthal_meta;
  /// Filename of the text file containing the control drum positions
  const std::string _position_file_name;
  /// MeshMetaData: whether the peripheral area of the generated mesh can be trimmed by PolygonMeshTrimmer
  const bool & _square_peripheral_trimmability;
  /// MeshMetaData: whether the generated mesh can be trimmed through its center by PolygonMeshTrimmer
  const bool & _square_center_trimmability;
  /// a Boolean flag to tell PeripheralModifyGenerator that the input is valid
  const bool & _peripheral_modifier_compatible;
  /// Subdomain IDs of the peripheral regions
  std::vector<subdomain_id_type> _peripheral_block_ids;
  /// Subdomain Names of the peripheral regions
  std::vector<SubdomainName> _peripheral_block_names;
};
