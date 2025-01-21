//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TriInterWrapperWettedPerimIC.h"
#include "TriInterWrapperMesh.h"

registerMooseObject("SubChannelApp", TriInterWrapperWettedPerimIC);

InputParameters
TriInterWrapperWettedPerimIC::validParams()
{
  InputParameters params = TriSubChannelBaseIC::validParams();
  params.addClassDescription(
      "Computes wetted perimeter of inter-wrapper cells in a triangular subchannel lattice");
  return params;
}

TriInterWrapperWettedPerimIC::TriInterWrapperWettedPerimIC(const InputParameters & params)
  : TriInterWrapperBaseIC(params)
{
}

Real
TriInterWrapperWettedPerimIC::value(const Point & p)
{
  auto flat_to_flat = _mesh.getSideX();
  auto gap = _mesh.getDuctToRodGap();
  auto element_side = flat_to_flat * std::tan(libMesh::pi / 6.0);
  bool tight_side_bypass = _mesh.getIsTightSide();
  auto element_side_ext = (flat_to_flat + 2. * gap) * std::tan(libMesh::pi / 6.0);

  auto i = _mesh.getSubchannelIndexFromPoint(p);
  // given the channel number, i, it computes the wetted perimeter of the subchannel
  // based on the subchannel type: CENTER, EDGE or CORNER.
  auto subch_type = _mesh.getSubchannelType(i);
  if (subch_type == EChannelType::CENTER)
  {
    return 3.0 * element_side;
  }
  else if (subch_type == EChannelType::EDGE)
  {
    if (tight_side_bypass)
      return 5.0 * element_side;
    else
      return 3.0 * element_side + 2.0 * gap * std::tan(libMesh::pi / 6.0);
  }

  else
  {
    if (tight_side_bypass)
      return (element_side + element_side_ext);
    else
      return 2.0 * element_side + 2.0 * gap * std::tan(libMesh::pi / 6.0);
  }
}
