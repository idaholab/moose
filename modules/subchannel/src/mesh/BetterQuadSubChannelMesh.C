#include "BetterQuadSubChannelMesh.h"

#include <cmath>

#include "libmesh/edge_edge2.h"
#include "libmesh/unstructured_mesh.h"

registerMooseObject("SubChannelApp", BetterQuadSubChannelMesh);

InputParameters
BetterQuadSubChannelMesh::validParams()
{
  InputParameters params = BetterSubChannelMeshBase::validParams();
  params.addRequiredParam<unsigned int>("nx", "Number of channels in the x direction [-]");
  params.addRequiredParam<unsigned int>("ny", "Number of channels in the y direction [-]");
  params.addRequiredParam<Real>("gap", "Half gap between assemblies [m]");
  return params;
}

BetterQuadSubChannelMesh::BetterQuadSubChannelMesh(const InputParameters & params)
  : BetterSubChannelMeshBase(params),
    _nx(getParam<unsigned int>("nx")),
    _ny(getParam<unsigned int>("ny")),
    _n_channels(_nx * _ny),
    _n_gaps((_nx - 1) * _ny + (_ny - 1) * _nx),
    _gap(getParam<Real>("gap"))
{
  if (_nx < 2 && _ny < 2)
    mooseError(name(),
               ": The number of subchannels cannot be less than 2 in both directions (x and y). "
               "Smallest assembly allowed is either 2X1 or 1X2. ");
  // Resize the gap-to-channel and channel-to-gap maps.
  _gap_to_chan_map.resize(_n_gaps);
  _gapnodes.resize(_n_gaps);
  _chan_to_gap_map.resize(_n_channels);
  _sign_id_crossflow_map.resize(_n_channels);
  _gij_map.resize(_n_gaps);
  double possitive_flow = 1.0;
  double negative_flow = -1.0;

  _subch_type.resize(_n_channels);
  for (unsigned int iy = 0; iy < _ny; iy++)
  {
    for (unsigned int ix = 0; ix < _nx; ix++)
    {
      unsigned int i_ch = _nx * iy + ix;
      bool is_corner = (ix == 0 && iy == 0) || (ix == _nx - 1 && iy == 0) ||
                       (ix == 0 && iy == _ny - 1) || (ix == _nx - 1 && iy == _ny - 1);
      bool is_edge = (ix == 0 || iy == 0 || ix == _nx - 1 || iy == _ny - 1);

      if (is_corner)
        _subch_type[i_ch] = EChannelType::CORNER;
      else if (is_edge)
        _subch_type[i_ch] = EChannelType::EDGE;
      else
        _subch_type[i_ch] = EChannelType::CENTER;
    }
  }

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
        _gij_map[i_gap] = (_pitch - _rod_diameter) / 2 + _gap;
      else
        _gij_map[i_gap] = (_pitch - _rod_diameter);
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

BetterQuadSubChannelMesh::BetterQuadSubChannelMesh(const BetterQuadSubChannelMesh & other_mesh)
  : BetterSubChannelMeshBase(other_mesh),
    _nx(other_mesh._nx),
    _ny(other_mesh._ny),
    _n_channels(other_mesh._n_channels),
    _n_gaps(other_mesh._n_gaps),
    _gap(other_mesh._gap),
    _nodes(other_mesh._nodes),
    _gapnodes(other_mesh._gapnodes),
    _gap_to_chan_map(other_mesh._gap_to_chan_map),
    _chan_to_gap_map(other_mesh._chan_to_gap_map),
    _sign_id_crossflow_map(other_mesh._sign_id_crossflow_map),
    _gij_map(other_mesh._gij_map)
{
}

std::unique_ptr<MooseMesh>
BetterQuadSubChannelMesh::safeClone() const
{
  return libmesh_make_unique<BetterQuadSubChannelMesh>(*this);
}

void
BetterQuadSubChannelMesh::buildMesh()
{
  UnstructuredMesh & mesh = dynamic_cast<UnstructuredMesh &>(getMesh());
  mesh.clear();
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  mesh.set_spatial_dimension(3);
  mesh.reserve_elem(_n_cells * _ny * _nx);
  mesh.reserve_nodes((_n_cells + 1) * _ny * _nx);
  _nodes.resize(_nx * _ny);
  // Add the points in the shape of a rectilinear grid.  The grid is regular
  // on the xy-plane with a spacing of `pitch` between points.  The grid along
  // z is irregular to account for rod spacers.  Store pointers in the _nodes
  // array so we can keep track of which points are in which channels.
  Real offset_x = (_nx - 1) * _pitch / 2.0;
  Real offset_y = (_ny - 1) * _pitch / 2.0;
  unsigned int node_id = 0;
  for (unsigned int iy = 0; iy < _ny; iy++)
  {
    for (unsigned int ix = 0; ix < _nx; ix++)
    {
      int i_ch = _nx * iy + ix;
      _nodes[i_ch].reserve(_n_cells);
      for (unsigned int iz = 0; iz < _n_cells + 1; iz++)
      {
        _nodes[i_ch].push_back(mesh.add_point(
            Point(_pitch * ix - offset_x, _pitch * iy - offset_y, _z_grid[iz]), node_id++));
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
      for (unsigned int iz = 0; iz < _n_cells; iz++)
      {
        Elem * elem = new Edge2;
        elem->set_id(elem_id++);
        elem = mesh.add_elem(elem);
        const int indx1 = ((_n_cells + 1) * _nx) * iy + (_n_cells + 1) * ix + iz;
        const int indx2 = ((_n_cells + 1) * _nx) * iy + (_n_cells + 1) * ix + (iz + 1);
        elem->set_node(0) = mesh.node_ptr(indx1);
        elem->set_node(1) = mesh.node_ptr(indx2);

        if (iz == 0)
          boundary_info.add_side(elem, 0, 0);
        if (iz == _n_cells - 1)
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

unsigned int
BetterQuadSubChannelMesh::getSubchannelIndexFromPoint(const Point & p) const
{
  Real offset_x = (_nx - 1) * _pitch / 2.0;
  Real offset_y = (_ny - 1) * _pitch / 2.0;
  unsigned int i = (p(0) + offset_x + 0.5 * _pitch) / _pitch;
  unsigned int j = (p(1) + offset_y + 0.5 * _pitch) / _pitch;
  return j * _nx + i;
}
