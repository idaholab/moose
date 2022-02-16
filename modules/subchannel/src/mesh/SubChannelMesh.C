#include "SubChannelMesh.h"

InputParameters
SubChannelMesh::validParams()
{
  InputParameters params = MooseMesh::validParams();
  params.set<MooseEnum>("dim") = "3";
  return params;
}

SubChannelMesh::SubChannelMesh(const InputParameters & params) : MooseMesh(params) {}

SubChannelMesh::SubChannelMesh(const SubChannelMesh & other_mesh)
  : MooseMesh(other_mesh),
    _unheated_length_entry(other_mesh._unheated_length_entry),
    _heated_length(other_mesh._heated_length),
    _unheated_length_exit(other_mesh._unheated_length_exit),
    _z_grid(other_mesh._z_grid),
    _k_grid(other_mesh._k_grid),
    _spacer_z(other_mesh._spacer_z),
    _spacer_k(other_mesh._spacer_k),
    _kij(other_mesh._kij),
    _pitch(other_mesh._pitch),
    _rod_diameter(other_mesh._rod_diameter),
    _n_cells(other_mesh._n_cells),
    _n_blocks(other_mesh._n_blocks)
{
}

unsigned int
SubChannelMesh::getZIndex(const Point & point) const
{
  if (_z_grid.size() == 0)
    mooseError("_z_grid is empty.");

  if (point(2) <= _z_grid[0])
    return 0;
  if (point(2) >= _z_grid[_z_grid.size() - 1])
    return _z_grid.size() - 1;

  unsigned int lo = 0;
  unsigned int hi = _z_grid.size();
  while (lo < hi)
  {
    unsigned int mid = (lo + hi) / 2;
    if (std::abs(_z_grid[mid] - point(2)) < 1e-5)
      return mid;
    else if (_z_grid[mid] < point(2))
      lo = mid;
    else
      hi = mid;
  }
  return lo;
}
