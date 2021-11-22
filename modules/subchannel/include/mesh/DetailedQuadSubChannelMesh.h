#pragma once

#include <vector>
#include "MooseMesh.h"
#include "SubChannelEnums.h"

/**
 * Base class for detailed subchannel meshes
 */
class DetailedQuadSubChannelMesh : public MooseMesh
{
public:
  DetailedQuadSubChannelMesh(const InputParameters & parameters);
  DetailedQuadSubChannelMesh(const DetailedQuadSubChannelMesh & other_mesh);
  virtual std::unique_ptr<MooseMesh> safeClone() const override;
  virtual void buildMesh() override;
  EChannelType getSubchannelType(unsigned int index) const { return _subch_type[index]; }

protected:
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
  Real _rod_diameter;
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

public:
  static InputParameters validParams();
};
