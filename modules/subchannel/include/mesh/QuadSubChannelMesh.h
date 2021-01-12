#pragma once

#include "SubChannelMeshBase.h"

class QuadSubChannelMesh : public SubChannelMeshBase
{
public:
  QuadSubChannelMesh(const InputParameters & parameters);
  QuadSubChannelMesh(const QuadSubChannelMesh & other_mesh);
  virtual std::unique_ptr<MooseMesh> safeClone() const override;
  virtual void buildMesh() override;

  virtual const std::vector<std::vector<Node *>> & getNodes() const override { return _nodes; }
  virtual const unsigned int & getNumOfChannels() const override { return _n_channels; }
  virtual const unsigned int & getNumOfGapsPerLayer() const override { return _n_gaps; }
  virtual const std::vector<std::pair<unsigned int, unsigned int>> &
  getGapToChannelMap() const override
  {
    return _gap_to_chan_map;
  }
  virtual const std::vector<std::vector<unsigned int>> & getChannelToGapMap() const override
  {
    return _chan_to_gap_map;
  }
  virtual const std::vector<double> & getGapMap() const override { return _gij_map; }
  virtual const Real & getPitch() const override { return _pitch; }
  virtual const std::vector<std::vector<double>> & getSignCrossflowMap() const override
  {
    return _sign_id_crossflow_map;
  }

  const unsigned int & getNx() const { return _nx; }
  const unsigned int & getNy() const { return _ny; }

  const Real & getGap() const { return _gap; }

  unsigned int getSubchannelIndexFromPoint(const Point & p) const override;

  virtual EChannelType getSubchannelType(unsigned int index) const override
  {
    return _subch_type[index];
  }

protected:
  unsigned int _nx;
  unsigned int _ny;
  unsigned int _n_channels;
  /// Number of gaps per layer
  unsigned int _n_gaps;
  Real _gap;
  std::vector<std::vector<Node *>> _nodes;
  std::vector<std::vector<Node *>> _gapnodes;
  std::vector<std::pair<unsigned int, unsigned int>> _gap_to_chan_map;
  std::vector<std::vector<unsigned int>> _chan_to_gap_map;
  /// matrix
  std::vector<std::vector<double>> _sign_id_crossflow_map;
  /// vector map of gap size (m)
  std::vector<double> _gij_map;
  Real _max_dz;
  /// subchannel type
  std::vector<EChannelType> _subch_type;

public:
  static InputParameters validParams();
};
