#pragma once

#include "MeshGenerator.h"
#include "SubChannelEnums.h"

/**
 * Mesh class for triangular, edge and corner subchannels for hexagonal lattice fuel assemblies
 */
class TriSubChannelMeshGenerator : public MeshGenerator
{
public:
  TriSubChannelMeshGenerator(const InputParameters & parameters);

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
  std::vector<Real> _k_grid;
  /// axial location of the spacers
  const std::vector<Real> & _spacer_z;
  /// form loss coefficient of the spacers
  const std::vector<Real> & _spacer_k;
  /// Distance between the neighbor fuel rods, pitch
  Real _pitch;
  /// fuel rod diameter
  Real _rod_diameter;
  /// number of axial cells
  unsigned int _n_cells;
  /// number of rings of fuel rods
  unsigned int _n_rings;
  /// number of subchannels
  unsigned int _n_channels;
  /// the distance between flat surfaces of the duct facing each other
  Real _flat_to_flat;
  /// wire diameter
  Real _dwire;
  /// wire lead length
  Real _hwire;
  /// the gap thickness between the duct and peripheral fuel rods
  Real _duct_to_rod_gap;

  /// nodes
  std::vector<std::vector<Node *>> _nodes;
  /// stores the channel pairs for each gap
  std::vector<std::pair<unsigned int, unsigned int>> _gap_to_chan_map;
  /// stores the gaps that forms each subchannel
  std::vector<std::vector<unsigned int>> _chan_to_gap_map;
  /// Defines the global cross-flow direction -1 or 1 for each subchannel and
  /// for all gaps that are belonging to the corresponding subchannel.
  /// Given a subchannel and a gap, if the neighbor subchannel index belonging to the same gap is lower,
  /// set it to -1, otherwise set it to 1.
  std::vector<std::vector<Real>> _sign_id_crossflow_map;
  /// gap size
  std::vector<Real> _gij_map;
  /// x,y coordinates of the subchannels
  std::vector<std::vector<Real>> _subchannel_position;
  /// x,y coordinates of the fuel rods
  std::vector<Point> _rod_position;
  /// fuel rods that are belonging to each ring
  std::vector<std::vector<Real>> _rods_in_rings;
  /// stores the fuel rods belonging to each subchannel
  std::vector<std::vector<unsigned int>> _subchannel_to_rod_map;
  /// stores the fuel rods belonging to each gap
  std::vector<std::vector<unsigned int>> _gap_to_rod_map;
  /// number of fuel rods
  unsigned int _nrods;
  /// number of gaps
  unsigned int _n_gaps;
  /// subchannel type
  std::vector<EChannelType> _subch_type;
  /// gap type
  std::vector<EChannelType> _gap_type;
  /// sweeping flow model gap pairs per channel to specify directional edge flow
  std::vector<std::pair<unsigned int, unsigned int>> _gap_pairs_sf;
  /// sweeping flow model channel pairs to specify directional edge flow
  std::vector<std::pair<unsigned int, unsigned int>> _chan_pairs_sf;

public:
  static InputParameters validParams();
};
