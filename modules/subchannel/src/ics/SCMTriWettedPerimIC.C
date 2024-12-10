//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMTriWettedPerimIC.h"
#include "TriSubChannelMesh.h"

registerMooseObject("SubChannelApp", SCMTriWettedPerimIC);

InputParameters
SCMTriWettedPerimIC::validParams()

{
  InputParameters params = TriSubChannelBaseIC::validParams();
  params.addClassDescription(
      "Computes wetted perimeter of subchannels in a triangular lattice arrangement");
  return params;
}

SCMTriWettedPerimIC::SCMTriWettedPerimIC(const InputParameters & params)
  : TriSubChannelBaseIC(params)
{
}

Real
SCMTriWettedPerimIC::value(const Point & p)
{
  // Define geometry parameters.
  auto pitch = _mesh.getPitch();
  auto pin_diameter = _mesh.getPinDiameter();
  auto wire_diameter = _mesh.getWireDiameter();
  auto wire_lead_length = _mesh.getWireLeadLength();
  auto rod_circumference = libMesh::pi * pin_diameter;
  auto wire_circumference = libMesh::pi * wire_diameter;
  auto gap = _mesh.getDuctToRodGap();
  auto theta = std::acos(wire_lead_length /
                         std::sqrt(std::pow(wire_lead_length, 2) +
                                   std::pow(libMesh::pi * (pin_diameter + wire_diameter), 2)));
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
    return 0.5 * rod_circumference + 0.5 * wire_circumference / std::cos(theta) + pitch;
  }
  else
  {
    return (rod_circumference + wire_circumference / std::cos(theta)) / 6.0 +
           2.0 / std::sqrt(3.0) * (pin_diameter / 2.0 + gap);
  }
}
