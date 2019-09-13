#ifndef SUBCHANNELMESH_H
#define SUBCHANNELMESH_H

#include <vector>

#include "MooseMesh.h"

class SubchannelMesh;

template <>
InputParameters validParams<SubchannelMesh>();

class SubchannelMesh : public MooseMesh
{
public:
  SubchannelMesh(const InputParameters & parameters);

  virtual std::unique_ptr<MooseMesh> safeClone() const override;

  virtual void buildMesh() override;

  unsigned int nx_, ny_, nz_, n_channels_;

  Real pitch_;
  std::vector<Real> z_grid_;

  std::vector<std::vector<Node *>> nodes_;

  std::vector<std::pair<unsigned int, unsigned int>> gap_to_chan_map_;
  std::vector<std::vector<unsigned int>> chan_to_gap_map_;

protected:
  Real _max_dz;
};
#endif // SUBCHANNELMESH_H
