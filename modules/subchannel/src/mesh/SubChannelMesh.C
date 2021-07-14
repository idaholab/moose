#include "SubChannelMesh.h"

InputParameters
SubChannelMesh::validParams()
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

SubChannelMesh::SubChannelMesh(const InputParameters & params)
  : MooseMesh(params),
    _heated_length(getParam<Real>("heated_length")),
    _spacer_z(getParam<std::vector<Real>>("spacer_z")),
    _spacer_k(getParam<std::vector<Real>>("spacer_k")),
    _pitch(getParam<Real>("pitch")),
    _rod_diameter(getParam<Real>("rod_diameter")),
    _n_cells(getParam<unsigned int>("n_cells")),
    _n_blocks(getParam<unsigned int>("n_blocks"))
{
  if (_spacer_z.size() != _spacer_k.size())
    mooseError(name(), " : Size of vector spacer_z should be equal to size of vector spacer_k");

  if (_spacer_z.back() > _heated_length)
    mooseError(name(), " : Location of spacers should be less than the heated length");

  Real dz = _heated_length / _n_cells;
  for (unsigned int i = 0; i < _n_cells + 1; i++)
    _z_grid.push_back(dz * i);

  std::vector<int> spacer_cell;
  for (const auto & elem : _spacer_z)
    spacer_cell.emplace_back(std::round(elem * _n_cells / _heated_length));

  _k_grid.resize(_n_cells + 1, 0.0);

  for (unsigned int index = 0; index < spacer_cell.size(); index++)
    _k_grid[spacer_cell[index]] += _spacer_k[index];
}

SubChannelMesh::SubChannelMesh(const SubChannelMesh & other_mesh)
  : MooseMesh(other_mesh),
    _heated_length(other_mesh._heated_length),
    _z_grid(other_mesh._z_grid),
    _k_grid(other_mesh._k_grid),
    _spacer_z(other_mesh._spacer_z),
    _spacer_k(other_mesh._spacer_k),
    _pitch(other_mesh._pitch),
    _rod_diameter(other_mesh._rod_diameter),
    _n_cells(other_mesh._n_cells),
    _n_blocks(other_mesh._n_blocks)
{
}
