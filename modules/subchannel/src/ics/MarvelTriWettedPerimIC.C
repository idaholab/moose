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

#include "MarvelTriWettedPerimIC.h"
#include "TriSubChannelMesh.h"

registerMooseObject("SubChannelApp", MarvelTriWettedPerimIC);

InputParameters
MarvelTriWettedPerimIC::validParams()

{
  InputParameters params = TriSubChannelBaseIC::validParams();
  params.addClassDescription("Computes wetted perimeter of subchannels in a triangular lattice "
                             "arrangement in the marvel micro-reactor");
  return params;
}

MarvelTriWettedPerimIC::MarvelTriWettedPerimIC(const InputParameters & params)
  : TriSubChannelBaseIC(params)
{
}

Real
MarvelTriWettedPerimIC::value(const Point & p)
{
  // Define geometry parameters.
  auto rod_diameter = _mesh.getPinDiameter();
  auto wire_diameter = _mesh.getWireDiameter();
  auto wire_lead_length = _mesh.getWireLeadLength();
  auto rod_circumference = libMesh::pi * rod_diameter;
  auto wire_circumference = libMesh::pi * wire_diameter;
  auto gap = _mesh.getDuctToPinGap();
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
    return 0.5 * rod_circumference + 0.5 * wire_circumference / std::cos(theta) +
           2.0 * (libMesh::pi / 3.0) * (rod_diameter / 2.0 + gap);
  }
  else
  {
    return (rod_circumference + wire_circumference / std::cos(theta)) / 6.0 +
           (libMesh::pi / 3.0) * (rod_diameter / 2.0 + gap);
  }
}
