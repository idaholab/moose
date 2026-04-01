//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <vector>
#include <utility> // std::pair
#include <map>

#include "MooseMesh.h"
#include "SubChannelEnums.h"

/**
 * Base class for subchannel meshes
 */
class SubChannelMesh : public MooseMesh
{
public:
  SubChannelMesh(const InputParameters & parameters);
  SubChannelMesh(const SubChannelMesh & other_mesh);

  /**
   * Get axial location of layers
   */
  virtual const std::vector<Real> & getZGrid() const { return _z_grid; }

  /**
   * Get axial index of point
   */
  virtual unsigned int getZIndex(const Point & point) const;

  /**
   * Get axial cell location and value of loss coefficient
   */
  virtual const std::vector<std::vector<Real>> & getKGrid() const { return _k_grid; }

  /**
   * Get axial location of blockage (in,out) [m]
   */
  virtual const std::vector<Real> & getZBlockage() const { return _z_blockage; }

  /**
   * Get index of blocked subchannels
   */
  virtual const std::vector<unsigned int> & getIndexBlockage() const { return _index_blockage; }

  /**
   * Get area reduction of blocked subchannels
   */
  virtual const std::vector<Real> & getReductionBlockage() const { return _reduction_blockage; }

  /**
   * Return lateral loss coefficient
   */
  virtual const Real & getKij() const { return _kij; }

  /**
   * Return the number of axial cells
   */
  virtual unsigned int getNumOfAxialCells() const { return processor_id() == 0 ? _n_cells : 0; }

  /**
   * Get the subchannel mesh node for a given channel index and elevation index
   */
  virtual Node * getChannelNode(unsigned int i_chan, unsigned int iz) const = 0;

  /**
   * Get the pin mesh node for a given pin index and elevation index
   */
  virtual Node * getPinNode(unsigned int i_pin, unsigned int iz) const = 0;

  /**
   * Function that gets the channel node from the duct node
   */
  Node * getChannelNodeFromDuct(Node * duct_node) const;

  /**
   * Function that gets the duct node from the channel node
   */
  Node * getDuctNodeFromChannel(Node * channel_node) const;

  /**
   * Return the number of channels per layer
   */
  virtual unsigned int getNumOfChannels() const = 0;

  /**
   * Return if Pin Mesh exists or not
   */
  bool pinMeshExist() const { return _pin_mesh_exist; }

  /**
   * Return if Duct Mesh exists or not
   */
  bool ductMeshExist() const { return _duct_mesh_exist; }

  /**
   * Return the number of gaps per layer
   */
  virtual unsigned int getNumOfGapsPerLayer() const = 0;

  /**
   * Return the number of pins
   */
  virtual unsigned int getNumOfPins() const = 0;

  /**
   * Return a pair of subchannel indices for a given gap index
   */
  virtual const std::pair<unsigned int, unsigned int> &
  getGapChannels(unsigned int i_gap) const = 0;

  /**
   * Return a pair of pin indices for a given gap index
   */
  virtual const std::pair<unsigned int, unsigned int> & getGapPins(unsigned int i_gap) const = 0;

  /**
   * Return a vector of gap indices for a given channel index
   */
  virtual const std::vector<unsigned int> & getChannelGaps(unsigned int i_chan) const = 0;

  /**
   * Return a vector of channel indices for a given Pin index
   */
  virtual const std::vector<unsigned int> & getPinChannels(unsigned int i_pin) const = 0;

  /**
   * Return a vector of pin indices for a given channel index
   */
  virtual const std::vector<unsigned int> & getChannelPins(unsigned int i_chan) const = 0;

  /**
   * Return the pitch between 2 subchannels
   */
  virtual const Real & getPitch() const { return _pitch; }

  /**
   * Return Pin diameter
   */
  virtual const Real & getPinDiameter() const { return _pin_diameter; }

  /**
   * Return a sign for the crossflow given a subchannel index and local neighbor index
   */
  virtual const Real & getCrossflowSign(unsigned int i_chan, unsigned int i_local) const = 0;

  /**
   * Return unheated length at entry
   */
  virtual const Real & getHeatedLengthEntry() const { return _unheated_length_entry; }

  /**
   * Return heated length
   */
  virtual const Real & getHeatedLength() const { return _heated_length; }

  /**
   * Return unheated length at exit
   */
  virtual const Real & getHeatedLengthExit() const { return _unheated_length_exit; }

  /**
   * Return a subchannel index for a given physical point `p`
   */
  virtual unsigned int getSubchannelIndexFromPoint(const Point & p) const = 0;

  virtual unsigned int channelIndex(const Point & point) const = 0;

  /**
   * Return a pin index for a given physical point `p`
   */
  virtual unsigned int getPinIndexFromPoint(const Point & p) const = 0;

  virtual unsigned int pinIndex(const Point & p) const = 0;

  /**
   * Return the type of the subchannel for given subchannel index
   */
  virtual EChannelType getSubchannelType(unsigned int index) const = 0;

  /**
   * Return gap width for a given gap index
   */
  virtual Real getGapWidth(unsigned int axial_index, unsigned int gap_index) const = 0;

  /**
   * Function that returns the vector with the duct nodes
   *
   */
  const std::vector<Node *> & getDuctNodes() const { return _duct_nodes; }

  /**
   * Function that sets the channel-to-duct maps
   */
  void setChannelToDuctMaps(const std::vector<Node *> & duct_nodes);

  bool _pin_mesh_exist = false;
  bool _duct_mesh_exist = false;

protected:
  /// unheated length of the fuel Pin at the entry of the assembly
  Real _unheated_length_entry;
  /// heated length of the fuel Pin
  Real _heated_length;
  /// unheated length of the fuel Pin at the exit of the assembly
  Real _unheated_length_exit;

  /// axial location of nodes
  std::vector<Real> _z_grid;
  /// axial form loss coefficient per computational cell
  std::vector<std::vector<Real>> _k_grid;

  /// A list of all mesh nodes that form the (elements of) the duct
  /// mesh that surrounds the pins/subchannels.
  std::vector<Node *> _duct_nodes;

  /// Maps between channel nodes and duct nodes
  std::map<Node *, Node *> _chan_to_duct_node_map;
  std::map<Node *, Node *> _duct_node_to_chan_map;

  /// axial location of the spacers
  std::vector<Real> _spacer_z;
  /// form loss coefficient of the spacers
  std::vector<Real> _spacer_k;

  /// axial location of blockage (inlet, outlet) [m]
  std::vector<Real> _z_blockage;
  /// index of subchannels affected by blockage
  std::vector<unsigned int> _index_blockage;
  /// area reduction of subchannels affected by blockage
  std::vector<Real> _reduction_blockage;

  /// Lateral form loss coefficient
  Real _kij;
  /// Distance between the neighbor fuel pins, pitch
  Real _pitch;
  /// fuel Pin diameter
  Real _pin_diameter;

  /// number of axial cells
  unsigned int _n_cells;

public:
  /// x,y coordinates of the subchannel centroids
  std::vector<std::vector<Real>> _subchannel_position;

  static InputParameters validParams();

  /**
   * Generate the spacing in z-direction using heated and unteaded lengths
   */
  static void generateZGrid(Real unheated_length_entry,
                            Real heated_length,
                            Real unheated_length_exit,
                            unsigned int n_cells,
                            std::vector<Real> & z_grid);
};
