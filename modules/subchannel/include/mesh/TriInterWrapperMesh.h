#pragma once

#include "InterWrapperMesh.h"

/**
 * Mesh class for triangular, edge and corner inter_wrappers for hexagonal lattice fuel assemblies
 */
class TriInterWrapperMesh : public InterWrapperMesh
{
public:
  TriInterWrapperMesh(const InputParameters & parameters);
  TriInterWrapperMesh(const TriInterWrapperMesh & other_mesh);
  virtual std::unique_ptr<MooseMesh> safeClone() const override;
  virtual void buildMesh() override;

  virtual const unsigned int & getNumOfPins() const override { return _n_assemblies; }

  virtual Node * getPinNode(unsigned int, unsigned) const override { return nullptr; }

  virtual bool pinMeshExist() const override { return false; }

  virtual const Real & getDuctToRodGap() const { return _duct_to_rod_gap; }

  /**
   * Return the number of rods
   */
  virtual const unsigned int & getNumOfRods() const { return _n_assemblies; }

  /**
   * Return rod index given inter_wrapper index and local neighbor index
   */
  virtual const unsigned int & getRodIndex(const unsigned int channel_idx,
                                           const unsigned int neighbor_idx)
  {
    return _subchannel_to_rod_map[channel_idx][neighbor_idx];
  }

  virtual Node * getChannelNode(unsigned int i_chan, unsigned iz) const override
  {
    return _nodes[i_chan][iz];
  }
  virtual const unsigned int & getNumOfChannels() const override { return _n_channels; }
  virtual const unsigned int & getNumOfGapsPerLayer() const override { return _n_gaps; }
  virtual const std::pair<unsigned int, unsigned int> &
  getGapNeighborChannels(unsigned int i_gap) const override
  {
    return _gap_to_chan_map[i_gap];
  }
  virtual const std::vector<unsigned int> & getChannelGaps(unsigned int i_chan) const override
  {
    return _chan_to_gap_map[i_chan];
  }
  virtual const std::vector<double> & getGapMap() const override { return _gij_map; }
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

  virtual Real getGapWidth(unsigned int gap_index) const override { return _gij_map[gap_index]; }

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
    return _subchannel_to_rod_map[i_chan];
  }

  virtual unsigned int getPinIndexFromPoint(const Point & p) const override;
  virtual unsigned int pinIndex(const Point & p) const override;

  /**
   * Setup the internal maps when there is a outside duct present
   */
  void setChannelToDuctMaps(const std::vector<Node *> & duct_nodes);

protected:
  /// number of rings of fuel rods
  unsigned int _n_rings;
  /// number of subchannels
  unsigned int _n_channels;
  /// the distance between flat surfaces of the duct facing each other
  Real _flat_to_flat;
  /// the gap thickness between the duct and peripheral fuel rods
  Real _duct_to_rod_gap;
  /// nodes
  std::vector<std::vector<Node *>> _nodes;

  /// A list of all mesh nodes that form the (elements of) the hexagonal duct
  /// mesh that surrounds the rods/intertices.
  std::vector<Node *> _duct_nodes;
  /// A map for providing the closest/corresponding duct node associated
  /// with each interstice node. i.e. a map of interstice mesh nodes to duct mesh nodes.
  std::map<Node *, Node *> _chan_to_duct_node_map;
  /// A map for providing the closest/corresponding interstice node associated
  /// with each duct node. i.e. a map of duct mesh nodes to interstice mesh nodes.
  std::map<Node *, Node *> _duct_node_to_chan_map;

  /// stores the channel pairs for each gap
  std::vector<std::pair<unsigned int, unsigned int>> _gap_to_chan_map;
  /// stores the gaps that forms each interstice
  std::vector<std::vector<unsigned int>> _chan_to_gap_map;
  /// Defines the global cross-flow direction -1 or 1 for each interstice and
  /// for all gaps that are belonging to the corresponding interstice.
  /// Given a interstice and a gap, if the neighbor interstice index belonging to the same gap is lower,
  /// set it to -1, otherwise set it to 1.
  std::vector<std::vector<Real>> _sign_id_crossflow_map;
  /// gap size
  std::vector<Real> _gij_map;
  /// x,y coordinates of the interstice
  std::vector<std::vector<Real>> _subchannel_position;
  /// x,y coordinates of the fuel rods
  std::vector<Point> _rod_position;
  /// fuel rods that are belonging to each ring
  std::vector<std::vector<Real>> _rods_in_rings;
  /// stores the fuel rods belonging to each interstice
  std::vector<std::vector<unsigned int>> _subchannel_to_rod_map;
  /// stores the fuel rods belonging to each gap
  std::vector<std::vector<unsigned int>> _gap_to_rod_map;
  /// number of assemblies
  unsigned int _n_assemblies;
  /// number of gaps
  unsigned int _n_gaps;
  /// interstice type
  std::vector<EChannelType> _subch_type;
  /// gap type
  std::vector<EChannelType> _gap_type;
  /// sweeping flow model gap pairs per channel to specify directional edge flow
  std::vector<std::pair<unsigned int, unsigned int>> _gap_pairs_sf;
  /// sweeping flow model channel pairs to specify directional edge flow
  std::vector<std::pair<unsigned int, unsigned int>> _chan_pairs_sf;
  /// TODO: channel indices corresponding to a given pin index
  std::vector<std::vector<unsigned int>> _pin_to_chan_map;

public:
  static InputParameters validParams();

  /**
   * Calculates and stores the rod positions/centers for a hexagonal assembly
   * containing the given number of rings in a triangular/alternating row grid
   * spaced 'assembly_pitch' apart.  The points are generated such that the duct is
   * centered at the given center point.
   */
  static void
  rodPositions(std::vector<Point> & positions, unsigned int nrings, Real assembly_pitch, Point center);

  friend class TriInterWrapperMeshGenerator;

  /// number of corners in the duct x-sec
  static const unsigned int N_CORNERS = 6;
};
