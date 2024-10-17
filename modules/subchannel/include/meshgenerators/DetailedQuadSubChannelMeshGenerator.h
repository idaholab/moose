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

#include "MeshGenerator.h"
#include "SubChannelEnums.h"

/**
 * Mesh generator that builds a 3D mesh representing quadrilateral subchannels
 */
class DetailedQuadSubChannelMeshGenerator : public MeshGenerator
{
public:
  DetailedQuadSubChannelMeshGenerator(const InputParameters & parameters);
  virtual std::unique_ptr<MeshBase> generate() override;

protected:
  EChannelType getSubchannelType(unsigned int index) const { return _subch_type[index]; }
  std::vector<Real> getSubchannelPosition(unsigned int i) { return _subchannel_position[i]; }

  /// unheated length of the fuel rod at the entry of the assembly
  Real _unheated_length_entry;
  /// heated length of the fuel rod
  Real _heated_length;
  /// unheated length of the fuel rod at the exit of the assembly
  Real _unheated_length_exit;
  /// axial location of nodes
  std::vector<Real> _z_grid;
  /// Distance between the neighbor fuel rods, pitch
  Real _pitch;
  /// fuel rod diameter
  Real _pin_diameter;
  /// Number of cells in the axial direction
  unsigned int _n_cells;
  /// Number of subchannels in the x direction
  unsigned int _nx;
  /// Number of subchannels in the y direction
  unsigned int _ny;
  /// Total number of subchannels
  unsigned int _n_channels;
  /// Half of gap between adjacent assemblies
  Real _gap;
  /// Subchannel type
  std::vector<EChannelType> _subch_type;
  /// x,y coordinates of the subchannel centroids
  std::vector<std::vector<Real>> _subchannel_position;
  /// Subdomain ID used for the mesh block
  const unsigned int & _block_id;

public:
  static InputParameters validParams();
};
