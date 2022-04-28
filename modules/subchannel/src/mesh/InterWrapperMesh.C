#include "InterWrapperMesh.h"

InputParameters
InterWrapperMesh::validParams()
{
  InputParameters params = MooseMesh::validParams();
  params.set<MooseEnum>("dim") = "3";
  return params;
}

InterWrapperMesh::InterWrapperMesh(const InputParameters & params) : MooseMesh(params) {}

InterWrapperMesh::InterWrapperMesh(const InterWrapperMesh & other_mesh)
  : MooseMesh(other_mesh),
    _unheated_length_entry(other_mesh._unheated_length_entry),
    _heated_length(other_mesh._heated_length),
    _unheated_length_exit(other_mesh._unheated_length_exit),
    _z_grid(other_mesh._z_grid),
    _k_grid(other_mesh._k_grid),
    _kij(other_mesh._kij),
    _assembly_pitch(other_mesh._assembly_pitch),
    _assembly_side_x(other_mesh._assembly_side_x),
    _assembly_side_y(other_mesh._assembly_side_y),
    _n_cells(other_mesh._n_cells)
{
}

void
InterWrapperMesh::generateZGrid(Real unheated_length_entry,
                              Real heated_length,
                              Real unheated_length_exit,
                              unsigned int n_cells,
                              std::vector<Real> & z_grid)
{
  Real L = unheated_length_entry + heated_length + unheated_length_exit;
  Real dz = L / n_cells;
  for (unsigned int i = 0; i < n_cells + 1; i++)
    z_grid.push_back(dz * i);
}

unsigned int
InterWrapperMesh::getZIndex(const Point & point) const
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
