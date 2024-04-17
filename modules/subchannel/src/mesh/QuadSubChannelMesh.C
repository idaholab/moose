/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

#include "QuadSubChannelMesh.h"

#include <cmath>

#include "libmesh/edge_edge2.h"
#include "libmesh/unstructured_mesh.h"

registerMooseObject("SubChannelApp", QuadSubChannelMesh);

InputParameters
QuadSubChannelMesh::validParams()
{
  InputParameters params = SubChannelMesh::validParams();
  params.addClassDescription("Creates an subchannel mesh container in a square "
                             "lattice arrangement");
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
    _gap_to_pin_map(other_mesh._gap_to_pin_map),
    _chan_to_gap_map(other_mesh._chan_to_gap_map),
    _chan_to_pin_map(other_mesh._chan_to_pin_map),
    _pin_to_chan_map(other_mesh._pin_to_chan_map),
    _sign_id_crossflow_map(other_mesh._sign_id_crossflow_map),
    _gij_map(other_mesh._gij_map),
    _pin_mesh_exist(other_mesh._pin_mesh_exist)
{
  _subchannel_position = other_mesh._subchannel_position;
  if (_nx < 2 && _ny < 2)
    mooseError(name(),
               ": The number of subchannels cannot be less than 2 in both directions (x and y). "
               "Smallest assembly allowed is either 2X1 or 1X2. ");
}

std::unique_ptr<MooseMesh>
QuadSubChannelMesh::safeClone() const
{
  return std::make_unique<QuadSubChannelMesh>(*this);
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

unsigned int
QuadSubChannelMesh::channelIndex(const Point & pt) const
{
  // this is identical to getSubchannelIndexFromPoint, but when it is given a point "outside" the
  // normal subchannel geometry (i.e. a point that lies in a gap around the lattice) we still report
  // a valid subchannel index this is needed for transferring the solution onto a visualization mesh

  Real offset_x = (_nx - 1) * _pitch / 2.0;
  Real offset_y = (_ny - 1) * _pitch / 2.0;
  int i = (pt(0) + offset_x + 0.5 * _pitch) / _pitch;
  int j = (pt(1) + offset_y + 0.5 * _pitch) / _pitch;

  i = std::max(0, i);
  i = std::min(i, (int)(_nx - 1));

  j = std::max(0, j);
  j = std::min(j, (int)(_ny - 1));

  return j * _nx + i;
}

unsigned int
QuadSubChannelMesh::getPinIndexFromPoint(const Point & p) const
{
  Real offset_x = (_nx - 2) * _pitch / 2.0;
  Real offset_y = (_ny - 2) * _pitch / 2.0;
  unsigned int i = (p(0) + offset_x) / _pitch;
  unsigned int j = (p(1) + offset_y) / _pitch;
  return j * (_nx - 1) + i;
}

unsigned int
QuadSubChannelMesh::pinIndex(const Point & p) const
{
  Real offset_x = (_nx - 2) * _pitch / 2.0;
  Real offset_y = (_ny - 2) * _pitch / 2.0;
  unsigned int i = (p(0) + offset_x) / _pitch;
  unsigned int j = (p(1) + offset_y) / _pitch;
  return j * (_nx - 1) + i;
}

void
QuadSubChannelMesh::generatePinCenters(
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
