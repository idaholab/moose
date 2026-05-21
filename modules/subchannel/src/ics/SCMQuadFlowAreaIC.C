//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMQuadFlowAreaIC.h"

#include "QuadSubChannelMesh.h"

registerMooseObject("SubChannelApp", SCMQuadFlowAreaIC);

InputParameters
SCMQuadFlowAreaIC::validParams()
{
  InputParameters params = QuadSubChannelBaseIC::validParams();
  params.addClassDescription(
      "Computes subchannel flow area in the square lattice subchannel arrangement");
  return params;
}

SCMQuadFlowAreaIC::SCMQuadFlowAreaIC(const InputParameters & params) : QuadSubChannelBaseIC(params)
{
}

Real
SCMQuadFlowAreaIC::value(const Point & p)
{
  auto i = _mesh.getSubchannelIndexFromPoint(p);
  return _mesh.getSubchannelFlowArea(i, p(2));
}
