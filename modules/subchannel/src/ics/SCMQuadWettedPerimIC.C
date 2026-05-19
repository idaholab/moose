//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  auto i = _mesh.getSubchannelIndexFromPoint(p);
  return _mesh.getSubchannelWettedPerimeter(i);
}
