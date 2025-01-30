//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"
#include "SubChannelEnums.h"

/**
 * Mesh generator that builds a 3D mesh representing triangular subchannels
 */
class SCMDetailedTriSubChannelMeshGenerator : public MeshGenerator
{
public:
  SCMDetailedTriSubChannelMeshGenerator(const InputParameters & parameters);
  virtual std::unique_ptr<MeshBase> generate() override;

protected:
  /// returns the type of the subchannel given the index
  EChannelType getSubchannelType(unsigned int index) const { return _subch_type[index]; }
  Point rotatePoint(Point b, Real theta);
  Point translatePoint(Point & b, Point & translation_vector);
  /// returns the position of pin given pin index
  Point getPinPosition(unsigned int i) { return _pin_position[i]; }
  /// returns the position of subchannel given pin index
  std::vector<Real> getSubchannelPosition(unsigned int i) { return _subchannel_position[i]; }
  /// returns the index of neighboring pins given subchannel index
  std::vector<unsigned int> getSubChannelPins(unsigned int i) { return _chan_to_pin_map[i]; }

  /// unheated length of the fuel Pin at the entry of the assembly
  Real _unheated_length_entry;
  /// heated length of the fuel Pin
  Real _heated_length;
  /// unheated length of the fuel Pin at the exit of the assembly
  Real _unheated_length_exit;
  /// axial location of nodes
  std::vector<Real> _z_grid;
  /// Distance between the neighbor fuel pins, pitch
  Real _pitch;
  /// fuel Pin diameter
  Real _pin_diameter;
  /// Number of rings in the geometry
  unsigned int _n_rings;
  /// Half of gap between adjacent assemblies
  Real _flat_to_flat;
  /// Subchannel type
  std::vector<EChannelType> _subch_type;
  /// x,y coordinates of the fuel pins
  std::vector<Point> _pin_position;
  /// x,y coordinates of the subchannels
  std::vector<std::vector<Real>> _subchannel_position;
  /// Subdomain ID used for the mesh block
  const unsigned int & _block_id;
  /// Number of cells in the axial direction
  unsigned int _n_cells;
  /// Number of pins
  unsigned int _nrods;
  /// fuel pins that are belonging to each ring
  std::vector<std::vector<unsigned int>> _pins_in_rings;
  /// map inner and outer rings
  std::map<unsigned int, Real> _orientation_map;
  /// number of subchannels
  unsigned int _n_channels;
  /// stores the fuel pins belonging to each subchannel
  std::vector<std::vector<unsigned int>> _chan_to_pin_map;
  /// Flag to print out the detailed mesh assembly and coordinates
  bool _verbose;

public:
  static InputParameters validParams();
};
