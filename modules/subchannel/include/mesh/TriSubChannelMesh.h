//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SubChannelMesh.h"

#include <map>
#include <vector>
#include <memory>  // std::unique_ptr
#include <utility> // std::pair

/**
 * Mesh class for triangular, edge and corner subchannels for hexagonal lattice fuel assemblies
 */
class TriSubChannelMesh : public SubChannelMesh
{
public:
  TriSubChannelMesh(const InputParameters & parameters);
  TriSubChannelMesh(const TriSubChannelMesh & other_mesh);

  std::unique_ptr<MooseMesh> safeClone() const override;
  void buildMesh() override;

  unsigned int getNumOfPins() const override { return processor_id() == 0 ? _npins : 0; }

  Node * getPinNode(unsigned int i_pin, unsigned int iz) const override
  {
    return _pin_nodes[i_pin][iz];
  }

  /**
   * Return the the gap thickness between the duct and peripheral fuel pins
   */
  const Real & getDuctToPinGap() const { return _duct_to_pin_gap; }

  /**
   * Return the number of rings
   */
  const unsigned int & getNumOfRings() const { return _n_rings; }

  /**
   * Return Pin index given subchannel index and local neighbor index
   */
  const unsigned int & getPinIndex(const unsigned int channel_idx, const unsigned int neighbor_idx)
  {
    return _chan_to_pin_map[channel_idx][neighbor_idx];
  }

  /**
   * Return wire diameter
   */
  const Real & getWireDiameter() const { return _dwire; }

  /**
   * Return flat to flat [m]
   */
  const Real & getFlatToFlat() const { return _flat_to_flat; }

  /**
   * Return the wire lead length
   */
  const Real & getWireLeadLength() const { return _hwire; }

  Node * getChannelNode(unsigned int i_chan, unsigned int iz) const override
  {
    return _nodes[i_chan][iz];
  }

  unsigned int getNumOfChannels() const override { return processor_id() == 0 ? _n_channels : 0; }

  unsigned int getNumOfGapsPerLayer() const override { return processor_id() == 0 ? _n_gaps : 0; }

  const std::pair<unsigned int, unsigned int> & getGapChannels(unsigned int i_gap) const override
  {
    return _gap_to_chan_map[i_gap];
  }

  const std::pair<unsigned int, unsigned int> & getGapPins(unsigned int i_gap) const override
  {
    return _gap_to_pin_map[i_gap];
  }

  const std::vector<unsigned int> & getChannelGaps(unsigned int i_chan) const override
  {
    return _chan_to_gap_map[i_chan];
  }

  const Real & getCrossflowSign(unsigned int i_chan, unsigned int i_local) const override
  {
    return _sign_id_crossflow_map[i_chan][i_local];
  }

  unsigned int getSubchannelIndexFromPoint(const Point & p) const override;
  unsigned int channelIndex(const Point & point) const override;

  EChannelType getSubchannelType(unsigned int index) const override { return _subch_type[index]; }

  Real getGapWidth(unsigned int axial_index, unsigned int gap_index) const override
  {
    return _gij_map[axial_index][gap_index];
  }

  const std::pair<unsigned int, unsigned int> & getSweepFlowGaps(unsigned int i_chan) const
  {
    return _gap_pairs_sf[i_chan];
  }

  const std::pair<unsigned int, unsigned int> & getSweepFlowChans(unsigned int i_chan) const
  {
    return _chan_pairs_sf[i_chan];
  }

  const std::vector<unsigned int> & getPinChannels(unsigned int i_pin) const override
  {
    return _pin_to_chan_map[i_pin];
  }

  const std::vector<unsigned int> & getChannelPins(unsigned int i_chan) const override
  {
    return _chan_to_pin_map[i_chan];
  }

  unsigned int getPinIndexFromPoint(const Point & p) const override;
  unsigned int pinIndex(const Point & p) const override;

protected:
  /// number of rings of fuel pins
  unsigned int _n_rings;
  /// number of subchannels
  unsigned int _n_channels;
  /// the distance between flat surfaces of the duct facing each other
  Real _flat_to_flat;
  /// wire diameter
  Real _dwire;
  /// wire lead length
  Real _hwire;
  /// the gap thickness between the duct and peripheral fuel pins
  Real _duct_to_pin_gap;

  /// nodes
  std::vector<std::vector<Node *>> _nodes;
  /// pin nodes
  std::vector<std::vector<Node *>> _pin_nodes;

  /// stores the channel pairs for each gap
  std::vector<std::pair<unsigned int, unsigned int>> _gap_to_chan_map;
  /// stores the fuel pins belonging to each gap
  std::vector<std::pair<unsigned int, unsigned int>> _gap_to_pin_map;
  /// stores the gaps that forms each subchannel
  std::vector<std::vector<unsigned int>> _chan_to_gap_map;

  /// Defines the global cross-flow direction -1 or 1 for each subchannel and
  /// for all gaps that are belonging to the corresponding subchannel.
  /// Given a subchannel and a gap, if the neighbor subchannel index belonging to the same gap is
  /// lower, set it to -1, otherwise set it to 1.
  std::vector<std::vector<Real>> _sign_id_crossflow_map;

  /// gap size
  std::vector<std::vector<Real>> _gij_map;

  /// x,y coordinates of the fuel pins
  std::vector<Point> _pin_position;

  /// fuel pins that are belonging to each ring
  std::vector<std::vector<Real>> _pins_in_rings;

  /// stores the fuel pins belonging to each subchannel
  std::vector<std::vector<unsigned int>> _chan_to_pin_map;

  /// number of fuel pins
  unsigned int _npins;
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

  /// channel indices corresponding to a given pin index
  std::vector<std::vector<unsigned int>> _pin_to_chan_map;

public:
  static InputParameters validParams();

  /**
   * Calculates and stores the pin positions/centers for a hexagonal assembly
   * containing the given number of rings in a triangular/alternating row grid
   * spaced 'pitch' apart. The points are generated such that the duct is
   * centered at the given center point.
   */
  static void
  pinPositions(std::vector<Point> & positions, unsigned int nrings, Real pitch, Point center);

  friend class SCMTriSubChannelMeshGenerator;
  friend class SCMTriDuctMeshGenerator;
  friend class SCMTriPinMeshGenerator;
  friend class SCMDetailedTriPinMeshGenerator;
  friend class TriSubChannel1PhaseProblem;

  /// number of corners in the duct x-sec
  static const unsigned int N_CORNERS = 6;
};
