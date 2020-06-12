#pragma once

#include <vector>
#include "MooseMesh.h"

class SubChannelMesh : public MooseMesh
{
public:
  SubChannelMesh(const InputParameters & parameters);

  virtual std::unique_ptr<MooseMesh> safeClone() const override;

  virtual void buildMesh() override;

  unsigned int nx_, ny_, nz_, n_channels_, n_gaps_;
  Real pitch_, rod_diameter_, gap_, heated_length_;
  std::vector<Real> z_grid_;

  std::vector<std::vector<Node *>> nodes_;
  std::vector<std::vector<Node *>> gapnodes_;

  std::vector<std::pair<unsigned int, unsigned int>> gap_to_chan_map_;
  std::vector<std::vector<unsigned int>> chan_to_gap_map_;
  /// matrix
  std::vector<std::vector<double>> sign_id_crossflow_map_;
  // vector map of gap size (m)
  std::vector<double> gij_map_;

protected:
  Real max_dz_;

public:
  static InputParameters validParams();
};
