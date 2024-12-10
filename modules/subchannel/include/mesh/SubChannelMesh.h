//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <vector>
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
  virtual const unsigned int & getNumOfAxialCells() const { return _n_cells; }

  /**
   * Get the subchannel mesh node for a given channel index and elevation index
   */
  virtual Node * getChannelNode(unsigned int i_chan, unsigned iz) const = 0;

  /**
   * Get the pin mesh node for a given pin index and elevation index
   */
  virtual Node * getPinNode(unsigned int i_pin, unsigned iz) const = 0;

  /**
   * Function that gets the channel node from the duct node
   */
  virtual Node * getChanNodeFromDuct(Node * duct_node) = 0;

  /**
   * Function that gets the channel node from the duct node
   */
  virtual Node * getChannelNodeFromDuct(Node * channel_node) = 0;

  /**
   * Function that gets the duct node from the channel node
   */
  virtual Node * getDuctNodeFromChannel(Node * channel_node) = 0;

  /**
   * Return the number of channels per layer
   */
  virtual const unsigned int & getNumOfChannels() const = 0;

  /**
   * Return if Pin Mesh exists or not
   */
  virtual bool pinMeshExist() const = 0;

  /**
   * Return if Duct Mesh exists or not
   */
  virtual bool ductMeshExist() const = 0;

  /**
   * Return the number of gaps per layer
   */
  virtual const unsigned int & getNumOfGapsPerLayer() const = 0;

  /**
   * Return the number of pins
   */
  virtual const unsigned int & getNumOfPins() const = 0;

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
   * Return rod diameter
   */
  virtual const Real & getPinDiameter() const { return _pin_diameter; }

  /**
   * Return a signs for the cross flow given a subchannel index and local neighbor index
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
   * Function that return the vector with the maps to the nodes if they exist
   */
  virtual const std::vector<Node *> getDuctNodes() const = 0;

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
  /// Distance between the neighbor fuel rods, pitch
  Real _pitch;
  /// fuel rod diameter
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
