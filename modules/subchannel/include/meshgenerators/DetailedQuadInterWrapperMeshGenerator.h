#pragma once

#include "MeshGenerator.h"
#include "SubChannelEnums.h"

/**
 * Mesh generator that builds a 3D mesh representing quadrilateral subchannels
 */
class DetailedQuadInterWrapperMeshGenerator : public MeshGenerator
{
public:
  DetailedQuadInterWrapperMeshGenerator(const InputParameters & parameters);
  virtual std::unique_ptr<MeshBase> generate() override;

protected:
  EChannelType getSubchannelType(unsigned int index) const { return _subch_type[index]; }

  /// unheated length of the fuel rod at the entry of the assembly
  Real _unheated_length_entry;
  /// heated length of the fuel rod
  Real _heated_length;
  /// unheated length of the fuel rod at the exit of the assembly
  Real _unheated_length_exit;
  /// axial location of nodes
  std::vector<Real> _z_grid;
  /// Distance between the neighbor fuel rods, pitch
  Real _assembly_pitch;
  /// Fuel assembly dimensions
  Real _assembly_side_x;
  Real _assembly_side_y;
  /// Number of cells in the axial direction
  unsigned int _n_cells;
  /// Number of subchannels in the x direction
  unsigned int _nx;
  /// Number of subchannels in the y direction
  unsigned int _ny;
  /// Total number of subchannels
  unsigned int _n_channels;
  /// Half of gap between adjacent assemblies
  Real _side_bypass_length;
  /// Subchannel type
  std::vector<EChannelType> _subch_type;
  /// Subdomain ID used for the mesh block
  const unsigned int & _block_id;

public:
  static InputParameters validParams();
};
