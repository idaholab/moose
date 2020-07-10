#pragma once

#include <vector>
#include "MooseMesh.h"

class SubChannelMesh : public MooseMesh
{
public:
  SubChannelMesh(const InputParameters & parameters);
  virtual std::unique_ptr<MooseMesh> safeClone() const override;
  virtual void buildMesh() override;
  unsigned int _nx, _ny, _nz, _n_channels, _n_gaps;
  Real _pitch, _rod_diameter, _gap, _heated_length;
  /// axial location of nodes
  std::vector<Real> _z_grid;
  /// axial locations of spacers
  const std::vector<Real> & _spacer_z;
  /// form loss coefficient of the spacer
  const std::vector<Real> & _spacer_k;
  std::vector<std::vector<Node *>> _nodes;
  std::vector<std::vector<Node *>> _gapnodes;
  std::vector<std::pair<unsigned int, unsigned int>> _gap_to_chan_map;
  std::vector<std::vector<unsigned int>> _chan_to_gap_map;
  /// matrix
  std::vector<std::vector<double>> _sign_id_crossflow_map;
  // vector map of gap size (m)
  std::vector<double> _gij_map;

protected:
  Real _max_dz;

public:
  static InputParameters validParams();
};
