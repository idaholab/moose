//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include <vector>
#include "MeshGenerator.h"
#include "libmesh/point.h"
#include "SubChannelEnums.h"

/**
 * Class for Subchannel mesh generation in the square lattice geometry
 */
class SCMQuadSubChannelMeshGenerator : public MeshGenerator
{
public:
  SCMQuadSubChannelMeshGenerator(const InputParameters & parameters);
  std::unique_ptr<MeshBase> generate() override;

protected:
  /// unheated length of the fuel Pin at the entry of the assembly
  const Real _unheated_length_entry;
  /// heated length of the fuel Pin
  const Real _heated_length;
  /// unheated length of the fuel Pin at the exit of the assembly
  const Real _unheated_length_exit;
  /// axial location of nodes
  std::vector<Real> _z_grid;
  /// axial form loss coefficient per computational cell
  std::vector<std::vector<Real>> _k_grid;
  /// axial location of the spacers
  const std::vector<Real> & _spacer_z;
  /// form loss coefficient of the spacers
  const std::vector<Real> & _spacer_k;
  /// axial location of blockage (inlet, outlet) [m]
  const std::vector<Real> _z_blockage;
  /// index of subchannels affected by blockage
  const std::vector<unsigned int> _index_blockage;
  /// area reduction of subchannels affected by blockage
  const std::vector<Real> _reduction_blockage;
  /// form loss coefficient of subchannels affected by blockage
  const std::vector<Real> _k_blockage;
  /// Lateral form loss coefficient
  const Real & _kij;
  /// Distance between the neighbor fuel pins, pitch
  const Real _pitch;
  /// fuel Pin diameter
  const Real _pin_diameter;
  /// number of axial cells
  const unsigned int _n_cells;
  /// number of axial blocks
  unsigned int _n_blocks;
  /// Number of subchannels in the x direction
  const unsigned int _nx;
  /// Number of subchannels in the y direction
  const unsigned int _ny;
  /// Total number of subchannels
  const unsigned int _n_channels;
  /// Number of gaps per layer
  const unsigned int _n_gaps;
  /// Number of pins
  const unsigned int _n_pins;
  /**
   * The gap, not to be confused with the gap between pins, this refers to the gap
   * next to the duct. Edge Pitch W = (pitch/2 - pin_diameter/2 + gap) [m]
   */
  const Real _gap;
  /// block index
  const unsigned int _block_id;
  /// Channel nodes
  std::vector<std::vector<Node *>> _nodes;
  /// gap nodes
  std::vector<std::vector<Node *>> _gapnodes;
  std::vector<std::pair<unsigned int, unsigned int>> _gap_to_chan_map;
  std::vector<std::pair<unsigned int, unsigned int>> _gap_to_pin_map;
  std::vector<std::vector<unsigned int>> _chan_to_gap_map;
  std::vector<std::vector<unsigned int>> _chan_to_pin_map;
  std::vector<std::vector<unsigned int>> _pin_to_chan_map;
  /// Matrix used to give local sign to crossflow quantities
  std::vector<std::vector<double>> _sign_id_crossflow_map;
  /// Vector to store gap size
  std::vector<std::vector<Real>> _gij_map;
  /// x,y coordinates of the subchannel centroid
  std::vector<std::vector<Real>> _subchannel_position;
  /// Subchannel type
  std::vector<EChannelType> _subch_type;

public:
  static InputParameters validParams();

  friend class SCMQuadPinMeshGenerator;
};
