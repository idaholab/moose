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

#include "SubChannelMesh.h"

/**
 * Mesh class for triangular, edge and corner subchannels for hexagonal lattice fuel assemblies
 */
class TriSubChannelMesh : public SubChannelMesh
{
public:
  TriSubChannelMesh(const InputParameters & parameters);
  TriSubChannelMesh(const TriSubChannelMesh & other_mesh);
  virtual std::unique_ptr<MooseMesh> safeClone() const override;
  virtual void buildMesh() override;

  virtual const unsigned int & getNumOfPins() const override { return _nrods; }

  virtual Node * getPinNode(unsigned int i_pin, unsigned iz) const override
  {
    return _pin_nodes[i_pin][iz];
  }

  virtual bool pinMeshExist() const override { return _pin_mesh_exist; }
  virtual bool ductMeshExist() const override { return _duct_mesh_exist; }

  virtual const Real & getDuctToRodGap() const { return _duct_to_pin_gap; }

  /**
   * Return the number of rods
   */
  virtual const unsigned int & getNumOfRods() const { return _nrods; }

  /**
   * Return rod index given subchannel index and local neighbor index
   */
  virtual const unsigned int & getRodIndex(const unsigned int channel_idx,
                                           const unsigned int neighbor_idx)
  {
    return _chan_to_pin_map[channel_idx][neighbor_idx];
  }

  /**
   * Return wire diameter
   */
  virtual const Real & getWireDiameter() const { return _dwire; }

  /**
   * Return the wire lead length
   */
  virtual const Real & getWireLeadLength() const { return _hwire; }

  virtual Node * getChannelNode(unsigned int i_chan, unsigned iz) const override
  {
    return _nodes[i_chan][iz];
  }
  virtual const unsigned int & getNumOfChannels() const override { return _n_channels; }
  virtual const unsigned int & getNumOfGapsPerLayer() const override { return _n_gaps; }
  virtual const std::pair<unsigned int, unsigned int> &
  getGapChannels(unsigned int i_gap) const override
  {
    return _gap_to_chan_map[i_gap];
  }
  virtual const std::pair<unsigned int, unsigned int> &
  getGapPins(unsigned int i_gap) const override
  {
    return _gap_to_pin_map[i_gap];
  }
  virtual const std::vector<unsigned int> & getChannelGaps(unsigned int i_chan) const override
  {
    return _chan_to_gap_map[i_chan];
  }
  virtual const Real & getCrossflowSign(unsigned int i_chan, unsigned int i_local) const override
  {
    return _sign_id_crossflow_map[i_chan][i_local];
  }

  virtual unsigned int getSubchannelIndexFromPoint(const Point & p) const override;
  virtual unsigned int channelIndex(const Point & point) const override;

  virtual EChannelType getSubchannelType(unsigned int index) const override
  {
    return _subch_type[index];
  }

  virtual Real getGapWidth(unsigned int axial_index, unsigned int gap_index) const override
  {
    return _gij_map[gap_index];
  }

  virtual const std::pair<unsigned int, unsigned int> & getSweepFlowGaps(unsigned int i_chan) const
  {
    return _gap_pairs_sf[i_chan];
  }

  virtual const std::pair<unsigned int, unsigned int> & getSweepFlowChans(unsigned int i_chan) const
  {
    return _chan_pairs_sf[i_chan];
  }

  virtual const std::vector<unsigned int> & getPinChannels(unsigned int i_pin) const override
  {
    return _pin_to_chan_map[i_pin];
  }

  virtual const std::vector<unsigned int> & getChannelPins(unsigned int i_chan) const override
  {
    return _chan_to_pin_map[i_chan];
  }

  virtual unsigned int getPinIndexFromPoint(const Point & p) const override;
  virtual unsigned int pinIndex(const Point & p) const override;

  /**
   * Function that gets the channel node from the duct node
   */
  void setChannelToDuctMaps(const std::vector<Node *> & duct_nodes);

  virtual Node * getChanNodeFromDuct(Node * duct_node) override
  {
    return _duct_node_to_chan_map[duct_node];
  }

  /**
   * Function that gets the duct node from the channel node
   */
  virtual Node * getDuctNodeFromChannel(Node * channel_node) override
  {
    return _chan_to_duct_node_map[channel_node];
  }

  /**
   * Function that gets the channel node from the duct node
   */
  virtual Node * getChannelNodeFromDuct(Node * channel_node) override
  {
    return _duct_node_to_chan_map[channel_node];
  }

  /**
   * Function that return the vector with the maps to the nodes
   */
  virtual const std::vector<Node *> getDuctNodes() const override { return _duct_nodes; }

protected:
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
  Real _duct_to_pin_gap;
  /// nodes
  std::vector<std::vector<Node *>> _nodes;
  /// pin nodes
  std::vector<std::vector<Node *>> _pin_nodes;

  /// A list of all mesh nodes that form the (elements of) the hexagonal duct
  /// mesh that surrounds the rods/subchannels.
  std::vector<Node *> _duct_nodes;
  /// A map for providing the closest/corresponding duct node associated
  /// with each subchannel node. i.e. a map of subchannel mesh nodes to duct mesh nodes.
  std::map<Node *, Node *> _chan_to_duct_node_map;
  /// A map for providing the closest/corresponding subchannel node associated
  /// with each duct node. i.e. a map of duct mesh nodes to subchannel mesh nodes.
  std::map<Node *, Node *> _duct_node_to_chan_map;

  /// stores the channel pairs for each gap
  std::vector<std::pair<unsigned int, unsigned int>> _gap_to_chan_map;
  /// stores the fuel pins belonging to each gap
  std::vector<std::pair<unsigned int, unsigned int>> _gap_to_pin_map;
  /// stores the gaps that forms each subchannel
  std::vector<std::vector<unsigned int>> _chan_to_gap_map;
  /// Defines the global cross-flow direction -1 or 1 for each subchannel and
  /// for all gaps that are belonging to the corresponding subchannel.
  /// Given a subchannel and a gap, if the neighbor subchannel index belonging to the same gap is lower,
  /// set it to -1, otherwise set it to 1.
  std::vector<std::vector<Real>> _sign_id_crossflow_map;
  /// gap size
  std::vector<Real> _gij_map;
  /// x,y coordinates of the fuel rods
  std::vector<Point> _pin_position;
  /// fuel rods that are belonging to each ring
  std::vector<std::vector<Real>> _pins_in_rings;
  /// stores the fuel rods belonging to each subchannel
  std::vector<std::vector<unsigned int>> _chan_to_pin_map;
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
  /// TODO: channel indices corresponding to a given pin index
  std::vector<std::vector<unsigned int>> _pin_to_chan_map;
  /// Flag that informs the solver whether there is a Pin Mesh or not
  bool _pin_mesh_exist;
  /// Flag that informs the solver whether there is a Duct Mesh or not
  bool _duct_mesh_exist;

public:
  static InputParameters validParams();

  /**
   * Calculates and stores the rod positions/centers for a hexagonal assembly
   * containing the given number of rings in a triangular/alternating row grid
   * spaced 'pitch' apart.  The points are generated such that the duct is
   * centered at the given center point.
   */
  static void
  rodPositions(std::vector<Point> & positions, unsigned int nrings, Real pitch, Point center);

  friend class TriSubChannelMeshGenerator;
  friend class TriDuctMeshGenerator;
  friend class TriPinMeshGenerator;
  friend class DetailedTriPinMeshGenerator;

  /// number of corners in the duct x-sec
  static const unsigned int N_CORNERS = 6;
};
