//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MarvelTriWettedPerimIC.h"
#include "TriSubChannelMesh.h"

registerMooseObject("SubChannelApp", MarvelTriWettedPerimIC);

InputParameters
MarvelTriWettedPerimIC::validParams()

{
  InputParameters params = TriSubChannelBaseIC::validParams();
  params.addClassDescription("Computes wetted perimeter of subchannels in a triangular lattice "
                             "arrangement in a MARVEL-type micro-reactor");
  return params;
}

MarvelTriWettedPerimIC::MarvelTriWettedPerimIC(const InputParameters & params)
  : TriSubChannelBaseIC(params)
{
}

Real
MarvelTriWettedPerimIC::value(const Point & p)
{
  // Define geometry parameters.
  auto pitch = _mesh.getPitch();
  auto rod_diameter = _mesh.getPinDiameter();
  auto wire_diameter = _mesh.getWireDiameter();
  auto wire_lead_length = _mesh.getWireLeadLength();
  auto rod_circumference = libMesh::pi * rod_diameter;
  auto wire_circumference = libMesh::pi * wire_diameter;
  auto gap = _mesh.getDuctToPinGap();
  auto r_ref = rod_diameter / 2.0 + gap;
  auto theta = std::acos(wire_lead_length /
                         std::sqrt(std::pow(wire_lead_length, 2) +
                                   std::pow(libMesh::pi * (rod_diameter + wire_diameter), 2)));
  // given the channel number, i, it computes the wetted perimeter of
  // the subchannel based on the subchannel type: CENTER, EDGE or CORNER.
  auto i = _mesh.getSubchannelIndexFromPoint(p);
  auto subch_type = _mesh.getSubchannelType(i);

  if (subch_type == EChannelType::CENTER)
  {
    return 0.5 * rod_circumference + 0.5 * wire_circumference / std::cos(theta);
  }
  else if (subch_type == EChannelType::EDGE)
  {
    auto gamma = std::acos(1 - 0.5 * std::pow(pitch / r_ref, 2.0));
    auto alpha = (libMesh::pi - gamma) / 2.0;
    auto sector_angle = libMesh::pi / 2.0 - alpha;
    return 0.5 * rod_circumference + 0.5 * wire_circumference / std::cos(theta) +
           2.0 * sector_angle * (r_ref);
  }
  else
  {
    return (rod_circumference + wire_circumference / std::cos(theta)) / 6.0 +
           (libMesh::pi / 3.0) * (r_ref);
  }
}
