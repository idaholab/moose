//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
 * Mesh generator that builds a 3D mesh representing quadrilateral subchannels
 */
class SCMDetailedQuadInterWrapperMeshGenerator : public MeshGenerator
{
public:
  SCMDetailedQuadInterWrapperMeshGenerator(const InputParameters & parameters);
  virtual std::unique_ptr<MeshBase> generate() override;

protected:
  EChannelType getSubchannelType(unsigned int index) const { return _subch_type[index]; }

  /// unheated length of the fuel Pin at the entry of the assembly
  const Real _unheated_length_entry;
  /// heated length of the fuel Pin
  const Real _heated_length;
  /// unheated length of the fuel Pin at the exit of the assembly
  const Real _unheated_length_exit;
  /// axial location of nodes
  std::vector<Real> _z_grid;
  /// Distance between the neighbor fuel pins, pitch
  const Real _assembly_pitch;
  /// Fuel assembly dimensions
  const Real _assembly_side_x;
  const Real _assembly_side_y;
  /// Number of cells in the axial direction
  const unsigned int _n_cells;
  /// Number of subchannels in the x direction
  const unsigned int _nx;
  /// Number of subchannels in the y direction
  const unsigned int _ny;
  /// Total number of subchannels
  unsigned int _n_channels;
  /// Half of gap between adjacent assemblies
  const Real _side_bypass_length;
  /// Subchannel type
  std::vector<EChannelType> _subch_type;
  /// Subdomain ID used for the mesh block
  const unsigned int & _block_id;

public:
  static InputParameters validParams();
};
