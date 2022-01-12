#include "QuadSubChannelMesh.h"

#include <cmath>

#include "libmesh/edge_edge2.h"
#include "libmesh/unstructured_mesh.h"

registerMooseObject("SubChannelApp", QuadSubChannelMesh);

InputParameters
QuadSubChannelMesh::validParams()
{
  InputParameters params = SubChannelMesh::validParams();
  return params;
}

QuadSubChannelMesh::QuadSubChannelMesh(const InputParameters & params)
  : SubChannelMesh(params), _pin_mesh_exist(false)
{
}

QuadSubChannelMesh::QuadSubChannelMesh(const QuadSubChannelMesh & other_mesh)
  : SubChannelMesh(other_mesh),
    _nx(other_mesh._nx),
    _ny(other_mesh._ny),
    _n_channels(other_mesh._n_channels),
    _n_gaps(other_mesh._n_gaps),
    _n_pins(other_mesh._n_pins),
    _gap(other_mesh._gap),
    _nodes(other_mesh._nodes),
    _gapnodes(other_mesh._gapnodes),
    _gap_to_chan_map(other_mesh._gap_to_chan_map),
    _chan_to_gap_map(other_mesh._chan_to_gap_map),
    _chan_to_pin_map(other_mesh._chan_to_pin_map),
    _pin_to_chan_map(other_mesh._pin_to_chan_map),
    _sign_id_crossflow_map(other_mesh._sign_id_crossflow_map),
    _gij_map(other_mesh._gij_map),
    _pin_mesh_exist(other_mesh._pin_mesh_exist)
{
  if (_nx < 2 && _ny < 2)
    mooseError(name(),
               ": The number of subchannels cannot be less than 2 in both directions (x and y). "
               "Smallest assembly allowed is either 2X1 or 1X2. ");
}

std::unique_ptr<MooseMesh>
QuadSubChannelMesh::safeClone() const
{
  return libmesh_make_unique<QuadSubChannelMesh>(*this);
}

void
QuadSubChannelMesh::buildMesh()
{
}

unsigned int
QuadSubChannelMesh::getSubchannelIndexFromPoint(const Point & p) const
{
  Real offset_x = (_nx - 1) * _pitch / 2.0;
  Real offset_y = (_ny - 1) * _pitch / 2.0;
  unsigned int i = (p(0) + offset_x + 0.5 * _pitch) / _pitch;
  unsigned int j = (p(1) + offset_y + 0.5 * _pitch) / _pitch;
  return j * _nx + i;
}
