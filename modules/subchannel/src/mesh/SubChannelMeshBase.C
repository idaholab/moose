#include "SubChannelMeshBase.h"

InputParameters
SubChannelMeshBase::validParams()
{
  InputParameters params = MooseMesh::validParams();
  params.set<MooseEnum>("dim") = "3";
  params.addRequiredParam<Real>("pitch", "Pitch [m]");
  params.addRequiredParam<Real>("rod_diameter", "Rod diameter [m]");
  params.addRequiredParam<Real>("heated_length", "Heated length [m]");
  params.addRequiredParam<std::vector<Real>>("spacer_z",
                                             "Axial location of spacers/vanes/mixing_vanes [m]");
  params.addRequiredParam<std::vector<Real>>(
      "spacer_k", "K-loss coefficient of spacers/vanes/mixing_vanes [-]");
  params.addRequiredParam<Real>("max_dz", "The maximum element height [m]");
  return params;
}

SubChannelMeshBase::SubChannelMeshBase(const InputParameters & params)
  : MooseMesh(params),
    _heated_length(getParam<Real>("heated_length")),
    _spacer_z(getParam<std::vector<Real>>("spacer_z")),
    _spacer_k(getParam<std::vector<Real>>("spacer_k")),
    _max_dz(getParam<Real>("max_dz")),
    _pitch(getParam<Real>("pitch")),
    _rod_diameter(getParam<Real>("rod_diameter"))
{
  // Define the node placement along the z-axis.
  std::vector<Real> block_sizes;
  if (_spacer_z.size() > 0 && _spacer_z[0] != 0)
  {
    block_sizes.push_back(_spacer_z[0]);
  }
  for (unsigned int i = 1; i < _spacer_z.size(); i++)
  {
    block_sizes.push_back(_spacer_z[i] - _spacer_z[i - 1]);
  }
  constexpr Real GRID_TOL = 1e-4;
  if (_spacer_z.size() > 0 && _spacer_z.back() < _heated_length - GRID_TOL)
  {
    block_sizes.push_back(_heated_length - _spacer_z.back());
  }
  _z_grid.push_back(0.0);
  for (auto block_size : block_sizes)
  {
    int n = 1;
    while (n * _max_dz < block_size)
      ++n;
    Real dz = block_size / n;
    for (int i = 0; i < n; i++)
      _z_grid.push_back(_z_grid.back() + dz);
  }
  _nz = _z_grid.size() - 1;
}

SubChannelMeshBase::SubChannelMeshBase(const SubChannelMeshBase & other_mesh)
  : MooseMesh(other_mesh),
    _nz(other_mesh._nz),
    _heated_length(other_mesh._heated_length),
    _z_grid(other_mesh._z_grid),
    _spacer_z(other_mesh._spacer_z),
    _spacer_k(other_mesh._spacer_k),
    _max_dz(other_mesh._max_dz),
    _pitch(other_mesh._pitch),
    _rod_diameter(other_mesh._rod_diameter)
{
}
