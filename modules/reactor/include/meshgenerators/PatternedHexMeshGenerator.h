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

class PatternedHexMeshGenerator;

template <>
InputParameters validParams<PatternedHexMeshGenerator>();

class PatternedHexMeshGenerator : public PolygonMeshGeneratorBase
{
public:
  static InputParameters validParams();

  PatternedHexMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  const std::vector<MeshGeneratorName> & _input_names;

  const std::vector<std::vector<unsigned int>> & _pattern;
  MooseEnum _pattern_boundary;
  const bool _generate_core_metadata;
  const unsigned int _background_intervals;
  const bool _has_assembly_duct;
  std::vector<Real> _duct_sizes;
  const enum class DuctStyle { apothem, radius } _duct_sizes_style;
  const std::vector<unsigned int> _duct_intervals;
  const bool _uniform_mesh_on_sides;
  const bool _generate_control_drum_positions_file;
  const bool _assign_control_drum_id;
  const Real _rotate_angle;
  const std::vector<subdomain_id_type> _duct_block_ids;
  const std::vector<SubdomainName> _duct_block_names;
  const boundary_id_type _external_boundary_id;
  const std::string _external_boundary_name;
  const enum class PolygonStyle { apothem, radius } _hexagon_size_style;
  Real _pattern_pitch;
  Real & _pattern_pitch_meta;
  const bool _is_control_drum_meta;
  std::vector<Point> & _control_drum_positions;
  std::vector<Real> & _control_drum_angles;
  std::vector<std::vector<Real>> & _control_drums_azimuthal_meta;
  const std::string _position_file_name;
  std::vector<std::unique_ptr<MeshBase> *> _mesh_ptrs;

  std::vector<subdomain_id_type> _peripheral_block_ids;
  std::vector<SubdomainName> _peripheral_block_names;

  std::vector<std::unique_ptr<ReplicatedMesh>> _meshes;

  std::unique_ptr<MeshBase> _out_meshes_ptrs;
  std::unique_ptr<ReplicatedMesh> _out_meshes;
  std::unique_ptr<ReplicatedMesh> _out_meshes_2;
};
