#pragma once

#include <vector>
#include "SubChannelMeshBase.h"

void rodPositions(std::vector<Point> & positions, Real nrings, Real pitch, Point center);

/**
 * Mesh class for triangular, edge and corner subchannels for hexagonal lattice fuel assemblies
 */
class TriSubChannelMesh : public SubChannelMeshBase
{
public:
  TriSubChannelMesh(const InputParameters & parameters);
  TriSubChannelMesh(const TriSubChannelMesh & other_mesh);
  virtual std::unique_ptr<MooseMesh> safeClone() const override;
  virtual void buildMesh() override;

  virtual const Real & getDuctToRodGap() const { return _duct_to_rod_gap; }

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

  virtual EChannelType getSubchannelType(unsigned int index) const override
  {
    return _subch_type[index];
  }

  virtual Real getGapWidth(unsigned int gap_index) const override { return _gij_map[gap_index]; }

protected:
  /// number of rings of fuel rods
  unsigned int _nrings;
  /// number of subchannels
  unsigned int _n_channels;
  /// the distance between flat surfaces of the duct facing each other
  Real _flat_to_flat;
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

public:
  static InputParameters validParams();
};
