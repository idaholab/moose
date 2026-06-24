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

/**
 * Mesh generator that builds a mesh of 1D lines representing subchannels and pins in a
 * quadrilateral assembly.
 */
class SCMQuadAssemblyMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  SCMQuadAssemblyMeshGenerator(const InputParameters & params);

  virtual std::unique_ptr<MeshBase> generate() override;

protected:
  /// Build subchannel, gap, pin, and cross-flow maps used by QuadSubChannelMesh.
  void initializeChannelData();
  /// Build the 1D subchannel elements and inlet/outlet boundaries.
  void buildSubchannelMesh(MeshBase & mesh_base, BoundaryInfo & boundary_info);
  /// Build the 1D pin elements for assemblies with pins.
  void buildPinMesh(MeshBase & mesh_base);
  /// Move generated mesh metadata into the concrete QuadSubChannelMesh object.
  void transferMetadata(QuadSubChannelMesh & sch_mesh);

protected:
  /// unheated length of the fuel Pin at the entry of the assembly
  Real _unheated_length_entry;
  /// heated length of the fuel Pin
  Real _heated_length;
  /// unheated length of the fuel Pin at the exit of the assembly
  Real _unheated_length_exit;

  /// axial location of the spacers
  std::vector<Real> _spacer_z;
  /// form loss coefficient of the spacers
  std::vector<Real> _spacer_k;
  /// axial location of blockage (inlet, outlet) [m]
  std::vector<Real> _z_blockage;
  /// index of subchannels affected by blockage
  std::vector<unsigned int> _index_blockage;
  /// area reduction of subchannels affected by blockage
  std::vector<Real> _reduction_blockage;
  /// form loss coefficient of subchannels affected by blockage
  std::vector<Real> _k_blockage;

  /// lateral form loss coefficient
  Real _kij;
  /// distance between neighbor fuel pins, pitch
  Real _pitch;
  /// fuel Pin diameter
  Real _pin_diameter;
  /// number of axial cells
  unsigned int _n_cells;
  /// number of subchannels in the x direction
  unsigned int _nx;
  /// number of subchannels in the y direction
  unsigned int _ny;
  /// total number of subchannels
  unsigned int _n_channels;
  /// number of gaps per layer
  unsigned int _n_gaps;
  /// number of pins
  unsigned int _n_pins;
  /**
   * The side gap, not to be confused with the gap between pins, this refers to the gap
   * next to the duct or else the distance between the subchannel centroid to the duct wall.
   * distance(edge pin center, duct wall) = pitch / 2 + side_gap [m].
   */
  Real _side_gap;

  /// subchannel block index
  unsigned int _subchannel_block_id;
  /// pin block index
  unsigned int _pin_block_id;

  /// axial location of nodes
  std::vector<Real> _z_grid;
  /// axial form loss coefficient per computational cell
  std::vector<std::vector<Real>> _k_grid;

  /// channel nodes
  std::vector<std::vector<Node *>> _nodes;
  /// pin nodes
  std::vector<std::vector<Node *>> _pin_nodes;
  /// gap nodes
  std::vector<std::vector<Node *>> _gapnodes;

  /// stores the channel pairs for each gap
  std::vector<std::pair<unsigned int, unsigned int>> _gap_to_chan_map;
  /// stores the fuel pin pairs for each gap
  std::vector<std::pair<unsigned int, unsigned int>> _gap_to_pin_map;
  /// stores the gaps that form each subchannel
  std::vector<std::vector<unsigned int>> _chan_to_gap_map;
  /// stores the fuel pins belonging to each subchannel
  std::vector<std::vector<unsigned int>> _chan_to_pin_map;
  /// stores the map from pins to channels
  std::vector<std::vector<unsigned int>> _pin_to_chan_map;
  /// matrix used to give local sign to crossflow quantities
  std::vector<std::vector<Real>> _sign_id_crossflow_map;
  /// gap size
  std::vector<std::vector<Real>> _gij_map;
  /// x,y coordinates of the subchannel centroids
  std::vector<std::vector<Real>> _subchannel_position;
  /// subchannel type
  std::vector<EChannelType> _subch_type;
};
