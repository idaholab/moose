#include "QuadInterWrapperMesh.h"

#include <cmath>

#include "libmesh/edge_edge2.h"
#include "libmesh/unstructured_mesh.h"

registerMooseObject("SubChannelApp", QuadInterWrapperMesh);

InputParameters
QuadInterWrapperMesh::validParams()
{
  InputParameters params = InterWrapperMesh::validParams();
  return params;
}

QuadInterWrapperMesh::QuadInterWrapperMesh(const InputParameters & params)
  : InterWrapperMesh(params), _pin_mesh_exist(false)
{
}

QuadInterWrapperMesh::QuadInterWrapperMesh(const QuadInterWrapperMesh & other_mesh)
  : InterWrapperMesh(other_mesh),
    _nx(other_mesh._nx),
    _ny(other_mesh._ny),
    _n_channels(other_mesh._n_channels),
    _n_gaps(other_mesh._n_gaps),
    _n_assemblies(other_mesh._n_assemblies),
    _side_bypass_length(other_mesh._side_bypass_length),
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
    mooseError(name(), ": The number of assemblies cannot be less than 1 in both directions. ");
}

std::unique_ptr<MooseMesh>
QuadInterWrapperMesh::safeClone() const
{
  return libmesh_make_unique<QuadInterWrapperMesh>(*this);
}

void
QuadInterWrapperMesh::buildMesh()
{
}

unsigned int
QuadInterWrapperMesh::getSubchannelIndexFromPoint(const Point & p) const
{
  Real offset_x = (_nx - 1) * _assembly_pitch / 2.0;
  Real offset_y = (_ny - 1) * _assembly_pitch / 2.0;
  unsigned int i = (p(0) + offset_x + 0.5 * _assembly_pitch) / _assembly_pitch;
  unsigned int j = (p(1) + offset_y + 0.5 * _assembly_pitch) / _assembly_pitch;
  return j * _nx + i;
}

unsigned int
QuadInterWrapperMesh::channelIndex(const Point & pt) const
{
  // this is identical to getSubchannelIndexFromPoint, but when it is given a point "outside" the
  // normal subchannel geometry (i.e. a point that lies in a gap around the lattice) we still report
  // a valid subchannel index this is needed for transferring the solution onto a visualization mesh

  Real offset_x = (_nx - 1) * _assembly_pitch / 2.0;
  Real offset_y = (_ny - 1) * _assembly_pitch / 2.0;
  int i = (pt(0) + offset_x + 0.5 * _assembly_pitch) / _assembly_pitch;
  int j = (pt(1) + offset_y + 0.5 * _assembly_pitch) / _assembly_pitch;

  i = std::max(0, i);
  i = std::min(i, (int)(_nx - 1));

  j = std::max(0, j);
  j = std::min(j, (int)(_ny - 1));

  return j * _nx + i;
}

unsigned int
QuadInterWrapperMesh::getPinIndexFromPoint(const Point & p) const
{
  Real offset_x = (_nx - 2) * _assembly_pitch / 2.0;
  Real offset_y = (_ny - 2) * _assembly_pitch / 2.0;
  unsigned int i = (p(0) + offset_x + 0.5 * _assembly_pitch) / _assembly_pitch;
  unsigned int j = (p(1) + offset_y + 0.5 * _assembly_pitch) / _assembly_pitch;
  return j * (_nx - 1) + i;
}

unsigned int
QuadInterWrapperMesh::pinIndex(const Point & p) const
{
  Real offset_x = (_nx - 2) * _assembly_pitch / 2.0;
  Real offset_y = (_ny - 2) * _assembly_pitch / 2.0;
  int i = (p(0) + offset_x + 0.5 * _assembly_pitch) / _assembly_pitch;
  int j = (p(1) + offset_y + 0.5 * _assembly_pitch) / _assembly_pitch;

  i = std::max(0, i);
  i = std::min(i, (int)(_nx - 2));

  j = std::max(0, j);
  j = std::min(j, (int)(_ny - 2));

  return j * (_nx - 1) + i;
}

void
QuadInterWrapperMesh::generatePinCenters(
    unsigned int nx, unsigned int ny, Real pitch, Real elev, std::vector<Point> & pin_centers)
{
  mooseAssert(nx >= 2, "Number of channels in x-direction must be 2 or more.");
  mooseAssert(ny >= 2, "Number of channels in y-direction must be 2 or more.");

  Real offset_x = (nx - 2) * pitch / 2.0;
  Real offset_y = (ny - 2) * pitch / 2.0;
  for (unsigned int iy = 0; iy < ny - 1; iy++)
    for (unsigned int ix = 0; ix < nx - 1; ix++)
      pin_centers.push_back(Point(pitch * ix - offset_x, pitch * iy - offset_y, elev));
}
