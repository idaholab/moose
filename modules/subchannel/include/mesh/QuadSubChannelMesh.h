#pragma once

#include "SubChannelMesh.h"
#include "SubChannelEnums.h"

class QuadSubChannelMesh : public SubChannelMesh
{
public:
  QuadSubChannelMesh(const InputParameters & parameters);
  QuadSubChannelMesh(const QuadSubChannelMesh & other_mesh);
  virtual std::unique_ptr<MooseMesh> safeClone() const override;
  virtual void buildMesh() override;

  virtual Node * getChannelNode(unsigned int i_chan, unsigned iz) const override
  {
    return _nodes[i_chan][iz];
  }

  virtual Node * getPinNode(unsigned int i_pin, unsigned iz) const override
  {
    return _pin_nodes[i_pin][iz];
  }

  virtual const unsigned int & getNumOfChannels() const override { return _n_channels; }
  virtual const unsigned int & getNumOfGapsPerLayer() const override { return _n_gaps; }
  virtual const unsigned int & getNumOfPins() const override { return _n_pins; }
  virtual bool pinMeshExist() const override { return _pin_mesh_exist; }
  virtual const std::pair<unsigned int, unsigned int> &
  getGapNeighborChannels(unsigned int i_gap) const override
  {
    return _gap_to_chan_map[i_gap];
  }
  virtual const std::vector<unsigned int> & getChannelGaps(unsigned int i_chan) const override
  {
    return _chan_to_gap_map[i_chan];
  }
  virtual const std::vector<unsigned int> & getPinChannels(unsigned int i_pin) const override
  {
    return _pin_to_chan_map[i_pin];
  }
  virtual const std::vector<unsigned int> & getChannelPins(unsigned int i_chan) const override
  {
    return _chan_to_pin_map[i_chan];
  }
  virtual const std::vector<double> & getGapMap() const override { return _gij_map; }
  virtual const Real & getPitch() const override { return _pitch; }
  virtual const Real & getCrossflowSign(unsigned int i_chan, unsigned int i_local) const override
  {
    return _sign_id_crossflow_map[i_chan][i_local];
  }

  virtual const unsigned int & getNx() const { return _nx; }
  virtual const unsigned int & getNy() const { return _ny; }
  const Real & getGap() const { return _gap; }

  unsigned int getSubchannelIndexFromPoint(const Point & p) const override;
  virtual unsigned int channelIndex(const Point & point) const override;

  unsigned int getPinIndexFromPoint(const Point & p) const override;
  virtual unsigned int pinIndex(const Point & p) const override;

  virtual EChannelType getSubchannelType(unsigned int index) const override
  {
    return _subch_type[index];
  }

  virtual Real getGapWidth(unsigned int gap_index) const override { return _gij_map[gap_index]; }

protected:
  unsigned int _nx;
  unsigned int _ny;
  unsigned int _n_channels;
  /// Number of gaps per layer
  unsigned int _n_gaps;
  /// Number of pins
  unsigned int _n_pins;
  Real _gap;
  std::vector<std::vector<Node *>> _nodes;
  std::vector<std::vector<Node *>> _pin_nodes;
  std::vector<std::vector<Node *>> _gapnodes;
  std::vector<std::pair<unsigned int, unsigned int>> _gap_to_chan_map;
  std::vector<std::vector<unsigned int>> _chan_to_gap_map;
  std::vector<std::vector<unsigned int>> _chan_to_pin_map;
  std::vector<std::vector<unsigned int>> _pin_to_chan_map;
  /// Matrix used to give local sign to crossflow quantities
  std::vector<std::vector<double>> _sign_id_crossflow_map;
  /// Vector to store gap size
  std::vector<double> _gij_map;
  /// Subchannel type
  std::vector<EChannelType> _subch_type;
  /// Flag that informs the solver whether there is a Pin Mesh or not
  bool _pin_mesh_exist;

public:
  static InputParameters validParams();

  /**
   * Generate pin centers
   *
   * @param nx number of channels in x-direction (must be more than 1)
   * @param ny number of channels in y-direction (must be more than 1)
   * @param elev elevation in z-direction
   * @param pin_centers Positions in 3D space of pin centers
   */
  static void generatePinCenters(
      unsigned int nx, unsigned int ny, Real pitch, Real elev, std::vector<Point> & pin_centers);

  friend class QuadSubChannelMeshGenerator;
  friend class QuadPinMeshGenerator;
};
