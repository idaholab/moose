/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

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
