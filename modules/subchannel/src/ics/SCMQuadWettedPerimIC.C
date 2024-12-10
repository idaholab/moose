//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMQuadWettedPerimIC.h"
#include "QuadSubChannelMesh.h"

registerMooseObject("SubChannelApp", SCMQuadWettedPerimIC);

InputParameters
SCMQuadWettedPerimIC::validParams()
{
  InputParameters params = QuadSubChannelBaseIC::validParams();
  params.addClassDescription(
      "Computes wetted perimeter of subchannels in a square lattice arrangement");
  return params;
}

SCMQuadWettedPerimIC::SCMQuadWettedPerimIC(const InputParameters & params)
  : QuadSubChannelBaseIC(params)
{
}

Real
SCMQuadWettedPerimIC::value(const Point & p)
{
  auto pitch = _mesh.getPitch();
  auto pin_diameter = _mesh.getPinDiameter();
  auto gap = _mesh.getGap();
  auto rod_circumference = M_PI * pin_diameter;
  auto i = _mesh.getSubchannelIndexFromPoint(p);
  auto subch_type = _mesh.getSubchannelType(i);

  if (subch_type == EChannelType::CORNER)
    return 0.25 * rod_circumference + pitch + 2 * gap;
  else if (subch_type == EChannelType::EDGE)
    return 0.5 * rod_circumference + pitch;
  else
    return rod_circumference;
}
