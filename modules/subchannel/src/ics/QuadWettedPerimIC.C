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

#include "QuadWettedPerimIC.h"
#include "QuadSubChannelMesh.h"

registerMooseObject("SubChannelApp", QuadWettedPerimIC);

InputParameters
QuadWettedPerimIC::validParams()
{
  InputParameters params = QuadSubChannelBaseIC::validParams();
  params.addClassDescription(
      "Computes wetted perimeter of subchannels in a square lattice arrangement");
  return params;
}

QuadWettedPerimIC::QuadWettedPerimIC(const InputParameters & params) : QuadSubChannelBaseIC(params)
{
}

Real
QuadWettedPerimIC::value(const Point & p)
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
