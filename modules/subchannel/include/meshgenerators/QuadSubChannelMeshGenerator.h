/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

#pragma once
#include <vector>
#include "MeshGenerator.h"
#include "libmesh/point.h"
#include "SubChannelEnums.h"

/**
 * Class for Subchannel mesh generation in the square lattice geometry
 */
class QuadSubChannelMeshGenerator : public MeshGenerator
{
public:
  QuadSubChannelMeshGenerator(const InputParameters & parameters);
  std::unique_ptr<MeshBase> generate() override;

protected:
  /// unheated length of the fuel rod at the entry of the assembly
  Real _unheated_length_entry;
  /// heated length of the fuel rod
  Real _heated_length;
  /// unheated length of the fuel rod at the exit of the assembly
  Real _unheated_length_exit;
  /// axial location of nodes
  std::vector<Real> _z_grid;
  /// axial form loss coefficient per computational cell
  std::vector<std::vector<Real>> _k_grid;
  /// axial location of the spacers
  const std::vector<Real> & _spacer_z;
  /// form loss coefficient of the spacers
  const std::vector<Real> & _spacer_k;
  /// axial location of blockage (inlet, outlet) [m]
  std::vector<Real> _z_blockage;
  /// index of subchannels affected by blockage
  std::vector<unsigned int> _index_blockage;
  /// area reduction of subchannels affected by blockage
  std::vector<Real> _reduction_blockage;
  /// form loss coefficient of subchannels affected by blockage
  std::vector<Real> _k_blockage;
  /// Lateral form loss coefficient
  const Real & _kij;
  /// Distance between the neighbor fuel rods, pitch
  Real _pitch;
  /// fuel rod diameter
  Real _pin_diameter;
  /// number of axial cells
  unsigned int _n_cells;
  /// number of axial blocks
  unsigned int _n_blocks;
  /// Number of subchannels in the x direction
  unsigned int _nx;
  /// Number of subchannels in the y direction
  unsigned int _ny;
  /// Total number of subchannels
  unsigned int _n_channels;
  /// Number of gaps per layer
  unsigned int _n_gaps;
  /// Number of pins
  unsigned int _n_pins;
  Real _gap;
  /// block index
  unsigned int _block_id;
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

  friend class QuadPinMeshGenerator;
};
