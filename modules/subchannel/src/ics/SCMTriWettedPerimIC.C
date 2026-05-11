//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  auto i = _mesh.getSubchannelIndexFromPoint(p);
  return _mesh.getSubchannelWettedPerimeter(i);
}
