#include "SubChannelMesh.h"

#include <cmath>

#include "libmesh/edge_edge2.h"
#include "libmesh/unstructured_mesh.h"

registerMooseObject("SubChannelApp", SubChannelMesh);

InputParameters
SubChannelMesh::validParams()
{
  InputParameters params = MooseMesh::validParams();
  params.addRequiredParam<unsigned int>("nx", "Number of channels in the x direction [-]");
  params.addRequiredParam<unsigned int>("ny", "Number of channels in the y direction [-]");
  params.addRequiredParam<Real>("max_dz", "The maximum element height [m]");
  params.addRequiredParam<Real>("pitch", "Pitch [m]");
  params.addRequiredParam<Real>("rod_diameter", "Rod diameter [m]");
  params.addRequiredParam<Real>("gap", "Half gap between assemblies [m]");
  params.addRequiredParam<Real>("heated_length", "Heated length [m]");
  params.addRequiredParam<std::vector<Real>>("spacer_z",
                                             "Axial location of spacers/vanes/mixing_vanes [m]");
  params.addRequiredParam<std::vector<Real>>(
      "spacer_k", "K-loss coefficient of spacers/vanes/mixing_vanes [-]");
  return params;
}

SubChannelMesh::SubChannelMesh(const InputParameters & params)
  : MooseMesh(params),
    _nx(getParam<unsigned int>("nx")),
    _ny(getParam<unsigned int>("ny")),
    _n_channels(_nx * _ny),
    _n_gaps((_nx - 1) * _ny + (_ny - 1) * _nx),
    _pitch(getParam<Real>("pitch")),
    _rod_diameter(getParam<Real>("rod_diameter")),
    _gap(getParam<Real>("gap")),
    _heated_length(getParam<Real>("heated_length")),
    _spacer_z(getParam<std::vector<Real>>("spacer_z")),
    _spacer_k(getParam<std::vector<Real>>("spacer_k")),
    _max_dz(getParam<Real>("max_dz"))
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

  // Resize the gap-to-channel and channel-to-gap maps.
  unsigned int n_gaps = (_nx - 1) * _ny + (_ny - 1) * _nx;
  _gap_to_chan_map.resize(n_gaps);
  _gapnodes.resize(n_gaps);
  _chan_to_gap_map.resize(_nx * _ny);
  _sign_id_crossflow_map.resize(_nx * _ny);
  _gij_map.resize(n_gaps);
  double possitive_flow = 1.0;
  double negative_flow = -1.0;

  // Index the east-west gaps.
  unsigned int i_gap = 0;
  for (unsigned int iy = 0; iy < _ny; iy++)
  {
    for (unsigned int ix = 0; ix < _nx - 1; ix++)
    {
      unsigned int i_ch = _nx * iy + ix;
      unsigned int j_ch = _nx * iy + (ix + 1);
      _gap_to_chan_map[i_gap] = {i_ch, j_ch};
      _chan_to_gap_map[i_ch].push_back(i_gap);
      _chan_to_gap_map[j_ch].push_back(i_gap);
      _sign_id_crossflow_map[i_ch].push_back(possitive_flow);
      _sign_id_crossflow_map[j_ch].push_back(negative_flow);

      // make a gap size map
      if (iy == 0 || iy == _ny - 1)
      {
        _gij_map[i_gap] = (_pitch - _rod_diameter) / 2 + _gap;
      }
      else
      {
        _gij_map[i_gap] = (_pitch - _rod_diameter);
      }

      ++i_gap;
    }
  }

  // Index the north-south gaps.
  for (unsigned int iy = 0; iy < _ny - 1; iy++)
  {
    for (unsigned int ix = 0; ix < _nx; ix++)
    {
      unsigned int i_ch = _nx * iy + ix;
      unsigned int j_ch = _nx * (iy + 1) + ix;
      _gap_to_chan_map[i_gap] = {i_ch, j_ch};
      _chan_to_gap_map[i_ch].push_back(i_gap);
      _chan_to_gap_map[j_ch].push_back(i_gap);
      _sign_id_crossflow_map[i_ch].push_back(possitive_flow);
      _sign_id_crossflow_map[j_ch].push_back(negative_flow);

      // make a gap size map
      if (ix == 0 || ix == _nx - 1)
      {
        _gij_map[i_gap] = (_pitch - _rod_diameter) / 2 + _gap;
      }
      else
      {
        _gij_map[i_gap] = (_pitch - _rod_diameter);
      }

      ++i_gap;
    }
  }

  // Reduce reserved memory in the channel-to-gap map.
  for (auto & gap : _chan_to_gap_map)
  {
    gap.shrink_to_fit();
  }
}

std::unique_ptr<MooseMesh>
SubChannelMesh::safeClone() const
{
  return libmesh_make_unique<SubChannelMesh>(*this);
}

void
SubChannelMesh::buildMesh()
{
  UnstructuredMesh & mesh = dynamic_cast<UnstructuredMesh &>(getMesh());
  mesh.clear();
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  mesh.set_spatial_dimension(3);
  mesh.reserve_elem(_nz * _ny * _nx);
  mesh.reserve_nodes((_nz + 1) * _ny * _nx);
  _nodes.resize(_nx * _ny);
  // Add the points in the shape of a rectilinear grid.  The grid is regular
  // on the xy-plane with a spacing of `pitch` between points.  The grid along
  // z is irregular to account for rod spacers.  Store pointers in the _nodes
  // array so we can keep track of which points are in which channels.
  unsigned int node_id = 0;
  for (unsigned int iy = 0; iy < _ny; iy++)
  {
    for (unsigned int ix = 0; ix < _nx; ix++)
    {
      int i_ch = _nx * iy + ix;
      _nodes[i_ch].reserve(_nz);
      for (unsigned int iz = 0; iz < _nz + 1; iz++)
      {
        _nodes[i_ch].push_back(
            mesh.add_point(Point(_pitch * ix, _pitch * iy, _z_grid[iz]), node_id++));
      }
    }
  }

  // Add the elements which in this case are 2-node edges that link each
  // subchannel's nodes vertically.
  unsigned int elem_id = 0;
  for (unsigned int iy = 0; iy < _ny; iy++)
  {
    for (unsigned int ix = 0; ix < _nx; ix++)
    {
      for (unsigned int iz = 0; iz < _nz; iz++)
      {
        Elem * elem = new Edge2;
        elem->set_id(elem_id++);
        elem = mesh.add_elem(elem);
        const int indx1 = ((_nz + 1) * _nx) * iy + (_nz + 1) * ix + iz;
        const int indx2 = ((_nz + 1) * _nx) * iy + (_nz + 1) * ix + (iz + 1);
        elem->set_node(0) = mesh.node_ptr(indx1);
        elem->set_node(1) = mesh.node_ptr(indx2);

        if (iz == 0)
          boundary_info.add_side(elem, 0, 0);
        if (iz == _nz - 1)
          boundary_info.add_side(elem, 1, 1);
      }
    }
  }

  boundary_info.sideset_name(0) = "inlet";
  boundary_info.sideset_name(1) = "outlet";
  boundary_info.nodeset_name(0) = "inlet";
  boundary_info.nodeset_name(1) = "outlet";
  mesh.prepare_for_use();
}
