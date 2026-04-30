//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "QuadSubChannelMesh.h"
#include <cmath>
#include <limits>
#include "libmesh/edge_edge2.h"
#include "libmesh/unstructured_mesh.h"

registerMooseObject("SubChannelApp", QuadSubChannelMesh);

InputParameters
QuadSubChannelMesh::validParams()
{
  InputParameters params = SubChannelMesh::validParams();
  params.addClassDescription("Creates an subchannel mesh container for a square "
                             "lattice arrangement");
  return params;
}

QuadSubChannelMesh::QuadSubChannelMesh(const InputParameters & params) : SubChannelMesh(params) {}

QuadSubChannelMesh::QuadSubChannelMesh(const QuadSubChannelMesh & other_mesh)
  : SubChannelMesh(other_mesh),
    _nx(other_mesh._nx),
    _ny(other_mesh._ny),
    _n_channels(other_mesh._n_channels),
    _n_gaps(other_mesh._n_gaps),
    _n_pins(other_mesh._n_pins),
    _side_gap(other_mesh._side_gap),
    _nodes(other_mesh._nodes),
    _pin_nodes(other_mesh._pin_nodes),
    _gapnodes(other_mesh._gapnodes),
    _gap_to_chan_map(other_mesh._gap_to_chan_map),
    _gap_to_pin_map(other_mesh._gap_to_pin_map),
    _chan_to_gap_map(other_mesh._chan_to_gap_map),
    _chan_to_pin_map(other_mesh._chan_to_pin_map),
    _pin_to_chan_map(other_mesh._pin_to_chan_map),
    _sign_id_crossflow_map(other_mesh._sign_id_crossflow_map),
    _gij_map(other_mesh._gij_map),
    _subch_type(other_mesh._subch_type)
{
  if (_nx < 2 && _ny < 2)
    mooseError(name(),
               ": The number of subchannels cannot be less than 2 in both directions (x and y). "
               "Smallest assembly allowed is either 2X1 or 1X2. ");
}

std::unique_ptr<MooseMesh>
QuadSubChannelMesh::safeClone() const
{
  return _app.getFactory().copyConstruct(*this);
}

void
QuadSubChannelMesh::buildMesh()
{
}

void
QuadSubChannelMesh::computeAssemblyHydraulicParameters()
{
  _assembly_flow_area = 0.0;
  _assembly_wetted_perimeter = 0.0;
  _assembly_hydraulic_diameter = 0.0;

  const Real z = _z_grid.empty() ? 0.0 : _z_grid.front();

  for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
  {
    _assembly_flow_area += getSubchannelFlowArea(i_ch, z);
    _assembly_wetted_perimeter += getSubchannelWettedPerimeter(i_ch);
  }

  if (_assembly_wetted_perimeter == 0.0)
    mooseError(name(), ": Assembly wetted perimeter is zero; cannot compute hydraulic diameter.");

  _assembly_hydraulic_diameter = 4.0 * _assembly_flow_area / _assembly_wetted_perimeter;
}

Real
QuadSubChannelMesh::getSubchannelFlowArea(unsigned int i_chan, Real z) const
{
  Real standard_area = 0.0;
  Real rod_area = 0.0;
  Real additional_area = 0.0;

  const auto subch_type = getSubchannelType(i_chan);
  if (subch_type == EChannelType::CORNER)
  {
    standard_area = 0.25 * _pitch * _pitch;
    rod_area = 0.25 * 0.25 * libMesh::pi * _pin_diameter * _pin_diameter;
    additional_area = _pitch * _side_gap + _side_gap * _side_gap;
  }
  else if (subch_type == EChannelType::EDGE)
  {
    standard_area = 0.5 * _pitch * _pitch;
    rod_area = 0.5 * 0.25 * libMesh::pi * _pin_diameter * _pin_diameter;
    additional_area = _pitch * _side_gap;
  }
  else
  {
    standard_area = _pitch * _pitch;
    rod_area = 0.25 * libMesh::pi * _pin_diameter * _pin_diameter;
  }

  Real flow_area = standard_area + additional_area - rod_area;

  unsigned int blockage_index = 0;
  for (const auto & i_blockage : _index_blockage)
  {
    if (i_chan == i_blockage && z >= _z_blockage.front() && z <= _z_blockage.back())
      flow_area *= _reduction_blockage[blockage_index];
    blockage_index++;
  }

  return flow_area;
}

Real
QuadSubChannelMesh::getSubchannelWettedPerimeter(unsigned int i_chan) const
{
  const Real rod_circumference = libMesh::pi * _pin_diameter;
  const auto subch_type = getSubchannelType(i_chan);

  if (subch_type == EChannelType::CORNER)
    return 0.25 * rod_circumference + _pitch + 2.0 * _side_gap;
  else if (subch_type == EChannelType::EDGE)
    return 0.5 * rod_circumference + _pitch;
  else
    return rod_circumference;
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
  if (_n_pins == 0)
    mooseError(name(), ": Cannot compute a pin index because this mesh has no pins.");

  Real offset_x = (_nx - 2) * _pitch / 2.0;
  Real offset_y = (_ny - 2) * _pitch / 2.0;
  unsigned int i = (p(0) + offset_x) / _pitch;
  unsigned int j = (p(1) + offset_y) / _pitch;
  return j * (_nx - 1) + i;
}

unsigned int
QuadSubChannelMesh::pinIndex(const Point & p) const
{
  if (_n_pins == 0)
    mooseError(name(), ": Cannot compute a pin index because this mesh has no pins.");

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
