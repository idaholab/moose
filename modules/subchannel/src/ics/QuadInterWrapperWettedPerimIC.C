//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "QuadInterWrapperWettedPerimIC.h"
#include "QuadInterWrapperMesh.h"

registerMooseObject("SubChannelApp", QuadInterWrapperWettedPerimIC);

InputParameters
QuadInterWrapperWettedPerimIC::validParams()
{
  InputParameters params = QuadSubChannelBaseIC::validParams();
  params.addClassDescription("Computes wetted perimeter of inter-wrapper cells in the square "
                             "lattice subchannel arrangement");
  return params;
}

QuadInterWrapperWettedPerimIC::QuadInterWrapperWettedPerimIC(const InputParameters & params)
  : QuadInterWrapperBaseIC(params)
{
}

Real
QuadInterWrapperWettedPerimIC::value(const Point & p)
{
  auto side_x = _mesh.getSideX();
  auto side_y = _mesh.getSideY();
  //  auto pitch = _mesh.getPitch();
  //  auto gap = _mesh.getGap();
  auto square_perimeter = 2.0 * side_x + 2.0 * side_y;
  auto i = _mesh.getSubchannelIndexFromPoint(p);
  auto subch_type = _mesh.getSubchannelType(i);

  if (subch_type == EChannelType::CORNER)
    // Need to consider specific side - will add later
    return 0.25 * square_perimeter;
  else if (subch_type == EChannelType::EDGE)
    // Need to consider specific side - will add later
    return 0.75 * square_perimeter;
  else
    return square_perimeter;
}
