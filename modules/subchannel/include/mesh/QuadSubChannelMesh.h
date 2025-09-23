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
#include "SubChannelEnums.h"

/**
 * Creates the mesh of subchannels in a quadrilateral lattice.
 */
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

  /**
   * Duct functions not applicable to quad channel
   *
   * Over-writing to avoid abstract template definition in this class
   */
  ///@{
  virtual Node * getDuctNodeFromChannel(Node *) override { return nullptr; }
  virtual Node * getChannelNodeFromDuct(Node *) override { return nullptr; }
  virtual const std::vector<Node *> getDuctNodes() const override { return std::vector<Node *>(); }
  ///@}

  virtual const unsigned int & getNumOfChannels() const override { return _n_channels; }
  virtual const unsigned int & getNumOfGapsPerLayer() const override { return _n_gaps; }
  virtual const unsigned int & getNumOfPins() const override { return _n_pins; }
  virtual bool pinMeshExist() const override { return _pin_mesh_exist; }
  virtual bool ductMeshExist() const override { return false; }
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
  virtual const std::vector<unsigned int> & getPinChannels(unsigned int i_pin) const override
  {
    return _pin_to_chan_map[i_pin];
  }
  virtual const std::vector<unsigned int> & getChannelPins(unsigned int i_chan) const override
  {
    return _chan_to_pin_map[i_chan];
  }
  virtual const Real & getPitch() const override { return _pitch; }
  virtual const Real & getCrossflowSign(unsigned int i_chan, unsigned int i_local) const override
  {
    return _sign_id_crossflow_map[i_chan][i_local];
  }
  /// Number of subchannels in the -x direction
  virtual const unsigned int & getNx() const { return _nx; }
  /// Number of subchannels in the -y direction
  virtual const unsigned int & getNy() const { return _ny; }
  /**
   * Returns the side gap, not to be confused with the gap between pins, this refers to the gap
   * next to the duct. Edge Pitch W = (pitch/2 - pin_diameter/2 + gap) [m]
   */
  const Real & getSideGap() const { return _side_gap; }

  unsigned int getSubchannelIndexFromPoint(const Point & p) const override;
  virtual unsigned int channelIndex(const Point & point) const override;

  unsigned int getPinIndexFromPoint(const Point & p) const override;
  virtual unsigned int pinIndex(const Point & p) const override;

  virtual EChannelType getSubchannelType(unsigned int index) const override
  {
    return _subch_type[index];
  }

  virtual Real getGapWidth(unsigned int axial_index, unsigned int gap_index) const override
  {
    return _gij_map[axial_index][gap_index];
  }

protected:
  /// number of subchannels in the x direction
  unsigned int _nx;
  /// number of subchannels in the y direction
  unsigned int _ny;
  /// number of subchannels in total
  unsigned int _n_channels;
  /// Number of gaps per layer
  unsigned int _n_gaps;
  /// Number of pins
  unsigned int _n_pins;
  /**
   * The side gap, not to be confused with the gap between pins, this refers to the gap
   * next to the duct or else the distance between the subchannel centroid to the duct wall.
   * Edge Pitch W = (pitch/2 - pin_diameter/2 + gap) [m]
   */
  Real _side_gap;
  /// vector of subchannel nodes
  std::vector<std::vector<Node *>> _nodes;
  /// vector of fuel pin nodes
  std::vector<std::vector<Node *>> _pin_nodes;
  /// vector of gap (interface between pairs of neighboring subchannels) nodes
  std::vector<std::vector<Node *>> _gapnodes;
  /// map relating gap index to subchannel index
  std::vector<std::pair<unsigned int, unsigned int>> _gap_to_chan_map;
  /// map relating gap index to fuel pin index
  std::vector<std::pair<unsigned int, unsigned int>> _gap_to_pin_map;
  /// map relating subchannel index to gap index
  std::vector<std::vector<unsigned int>> _chan_to_gap_map;
  /// map relating subchannel index to fuel pin index
  std::vector<std::vector<unsigned int>> _chan_to_pin_map;
  /// map relating fuel pin index to subchannel index
  std::vector<std::vector<unsigned int>> _pin_to_chan_map;
  /// Matrix used to give local sign to crossflow quantities
  std::vector<std::vector<double>> _sign_id_crossflow_map;
  /// Vector to store gap size
  std::vector<std::vector<Real>> _gij_map;
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

  friend class SCMQuadSubChannelMeshGenerator;
  friend class SCMQuadPinMeshGenerator;
  friend class QuadSubChannel1PhaseProblem;
};
