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
class QuadInterWrapperMeshGenerator : public MeshGenerator
{
public:
  QuadInterWrapperMeshGenerator(const InputParameters & parameters);
  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Distance between the neighbor fuel assemblies, assembly pitch
  /// Distance between the neighbor fuel assemblies, assembly pitch
  Real _assembly_pitch;
  /// Sides of the assemblies in the x and y direction
  Real _assembly_side_x;
  Real _assembly_side_y;
  /// unheated length of the inter-wrapper section at the entry of the assembly
  Real _unheated_length_entry;
  /// heated length of the inter-wrapper section
  Real _heated_length;
  /// unheated length of the inter-wrapper section at the exit of the assembly
  Real _unheated_length_exit;
  /// Lateral form loss coefficient
  const Real & _kij;
  /// number of axial cells
  unsigned int _n_cells;
  /// Number of assemblies in the x direction
  unsigned int _nx;
  /// Number of assemblies in the y direction
  unsigned int _ny;
  /// Extra bypass lengths in the sides of the assembly
  Real _side_bypass_length;
  /// Total number of flow channels
  unsigned int _n_channels;
  /// Number of gaps per layer
  unsigned int _n_gaps;
  /// Number of assemblies
  unsigned int _n_assemblies;
  /// block index
  unsigned int _block_id;

  /// axial location of nodes
  std::vector<Real> _z_grid;
  /// axial form loss coefficient per computational cell
  std::vector<std::vector<Real>> _k_grid;
  /// number of axial blocks
  unsigned int _n_blocks;

  // Note: this ideally needs to be changes
  // I am not changing them because it would entail changing the full solver
  /// Channel nodes
  std::vector<std::vector<Node *>> _nodes;
  /// Nodes of the gaps
  std::vector<std::vector<Node *>> _gapnodes;
  /// Defining the channel maps
  // Map from gaps to channels
  std::vector<std::pair<unsigned int, unsigned int>> _gap_to_chan_map;
  // Map from channel to gaps
  std::vector<std::vector<unsigned int>> _chan_to_gap_map;
  // Map of channels to pins
  std::vector<std::vector<unsigned int>> _chan_to_pin_map;
  // Map of pins to channels
  std::vector<std::vector<unsigned int>> _pin_to_chan_map;
  /// Matrix used to give local sign to crossflow quantities
  std::vector<std::vector<double>> _sign_id_crossflow_map;
  /// Vector to store gap size
  std::vector<double> _gij_map;
  /// Subchannel type
  std::vector<EChannelType> _subch_type;

public:
  static InputParameters validParams();

  friend class QuadPinMeshGenerator;
};
