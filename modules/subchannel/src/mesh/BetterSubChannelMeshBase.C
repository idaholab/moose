#include "BetterSubChannelMeshBase.h"

InputParameters
BetterSubChannelMeshBase::validParams()
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
  params.addRequiredParam<unsigned int>("n_cells", "The number of cells in the axial direction");
  params.addRequiredParam<unsigned int>("n_blocks", "The number of blocks in the axial direction");
  return params;
}

BetterSubChannelMeshBase::BetterSubChannelMeshBase(const InputParameters & params)
  : MooseMesh(params),
    _heated_length(getParam<Real>("heated_length")),
    _spacer_z(getParam<std::vector<Real>>("spacer_z")),
    _spacer_k(getParam<std::vector<Real>>("spacer_k")),
    _pitch(getParam<Real>("pitch")),
    _rod_diameter(getParam<Real>("rod_diameter")),
    _n_cells(getParam<unsigned int>("n_cells")),
    _n_blocks(getParam<unsigned int>("n_blocks"))
{
  _z_grid.push_back(0.0);
  Real _dz = _heated_length / _n_cells;
  for (int i = 0; i < _n_cells; i++)
    _z_grid.push_back(_z_grid.back() + _dz);
}

BetterSubChannelMeshBase::BetterSubChannelMeshBase(const BetterSubChannelMeshBase & other_mesh)
  : MooseMesh(other_mesh),
    _heated_length(other_mesh._heated_length),
    _z_grid(other_mesh._z_grid),
    _spacer_z(other_mesh._spacer_z),
    _spacer_k(other_mesh._spacer_k),
    _pitch(other_mesh._pitch),
    _rod_diameter(other_mesh._rod_diameter),
    _n_cells(other_mesh._n_cells),
    _n_blocks(other_mesh._n_blocks)
{
}
