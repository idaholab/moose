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
#include "QuadSubChannelMesh.h"

class SCMQuadAssemblyMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  SCMQuadAssemblyMeshGenerator(const InputParameters & params);

  virtual std::unique_ptr<MeshBase> generate() override;

protected:
  void initializeChannelData();
  void buildSubchannelMesh(MeshBase & mesh_base, BoundaryInfo & boundary_info);
  void buildPinMesh(MeshBase & mesh_base);
  void transferMetadata(QuadSubChannelMesh & sch_mesh);

protected:
  Real _unheated_length_entry;
  Real _heated_length;
  Real _unheated_length_exit;

  std::vector<Real> _spacer_z;
  std::vector<Real> _spacer_k;
  std::vector<Real> _z_blockage;
  std::vector<unsigned int> _index_blockage;
  std::vector<Real> _reduction_blockage;
  std::vector<Real> _k_blockage;

  Real _kij;
  Real _pitch;
  Real _pin_diameter;
  unsigned int _n_cells;
  unsigned int _nx;
  unsigned int _ny;
  unsigned int _n_channels;
  unsigned int _n_gaps;
  unsigned int _n_pins;
  Real _side_gap;

  unsigned int _subchannel_block_id;
  unsigned int _pin_block_id;

  std::vector<Real> _z_grid;
  std::vector<std::vector<Real>> _k_grid;

  std::vector<std::vector<Node *>> _nodes;
  std::vector<std::vector<Node *>> _pin_nodes;
  std::vector<std::vector<Node *>> _gapnodes;

  std::vector<std::pair<unsigned int, unsigned int>> _gap_to_chan_map;
  std::vector<std::pair<unsigned int, unsigned int>> _gap_to_pin_map;
  std::vector<std::vector<unsigned int>> _chan_to_gap_map;
  std::vector<std::vector<unsigned int>> _chan_to_pin_map;
  std::vector<std::vector<unsigned int>> _pin_to_chan_map;
  std::vector<std::vector<Real>> _sign_id_crossflow_map;
  std::vector<std::vector<Real>> _gij_map;
  std::vector<std::vector<Real>> _subchannel_position;
  std::vector<EChannelType> _subch_type;
};
