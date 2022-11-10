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

#include "TriWettedPerimIC.h"
#include "TriSubChannelMesh.h"

registerMooseObject("SubChannelApp", TriWettedPerimIC);

InputParameters
TriWettedPerimIC::validParams()
{
  return TriSubChannelBaseIC::validParams();
}

TriWettedPerimIC::TriWettedPerimIC(const InputParameters & params) : TriSubChannelBaseIC(params) {}

Real
TriWettedPerimIC::value(const Point & p)
{
  // Define geometry parameters.
  auto pitch = _mesh.getPitch();
  auto rod_diameter = _mesh.getRodDiameter();
  auto wire_diameter = _mesh.getWireDiameter();
  auto wire_lead_length = _mesh.getWireLeadLength();
  auto rod_circumference = libMesh::pi * rod_diameter;
  auto wire_circumference = libMesh::pi * wire_diameter;
  auto gap = _mesh.getDuctToRodGap();
  auto theta = std::acos(wire_lead_length /
                         std::sqrt(std::pow(wire_lead_length, 2) +
                                   std::pow(libMesh::pi * (rod_diameter + wire_diameter), 2)));
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
           2.0 / std::sqrt(3.0) * (rod_diameter / 2.0 + gap);
  }
}
