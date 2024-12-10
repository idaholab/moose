//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TriInterWrapperFlowAreaIC.h"
#include "TriInterWrapperMesh.h"

registerMooseObject("SubChannelApp", TriInterWrapperFlowAreaIC);

InputParameters
TriInterWrapperFlowAreaIC::validParams()
{
  InputParameters params = TriInterWrapperBaseIC::validParams();
  params.addClassDescription(
      "Computes flow area of inter-wrapper cells in a triangualar subchannel lattice");
  return params;
}

TriInterWrapperFlowAreaIC::TriInterWrapperFlowAreaIC(const InputParameters & params)
  : TriInterWrapperBaseIC(params)
{
}

Real
TriInterWrapperFlowAreaIC::value(const Point & p)
{
  auto pitch = _mesh.getPitch();
  auto flat_to_flat = _mesh.getSideX();
  auto gap = _mesh.getDuctToRodGap();
  auto element_side = flat_to_flat * std::tan(libMesh::pi / 6.0);
  auto tight_side_bypass = _mesh.getIsTightSide();
  auto i = _mesh.getSubchannelIndexFromPoint(p);
  // given the channel number, i, it computes the flow area of the subchannel
  // based on the subchannel type: CENTER, EDGE or CORNER.
  auto subch_type = _mesh.getSubchannelType(i);
  if (subch_type == EChannelType::CENTER)
  {
    return (pitch - flat_to_flat) * element_side * 3.0 / 2.0 +
           std::sin(libMesh::pi / 3.) * (pitch - flat_to_flat) * (pitch - flat_to_flat) / 2.0;
  }
  else if (subch_type == EChannelType::EDGE)
  {
    if (tight_side_bypass)
      return (pitch - flat_to_flat) * element_side * 0.5 + gap * element_side * 2 +
             gap * gap * std::tan(libMesh::pi / 6.0);
    else
      return (pitch - flat_to_flat) * element_side * 1.0 / 2.0 +
             (element_side + gap * std::tan(libMesh::pi / 6.0) / 2.0) * gap;
  }
  else
  {
    if (tight_side_bypass)
      return (element_side + gap * std::tan(libMesh::pi / 6.0)) * gap;
    else
      return (element_side + gap * std::tan(libMesh::pi / 6.0) / 2.0) * gap;
  }
}
