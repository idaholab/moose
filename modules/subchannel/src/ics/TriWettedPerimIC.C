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
  auto teta = acos(wire_lead_length / sqrt(pow(wire_lead_length, 2) +
                                           pow(libMesh::pi * (rod_diameter + wire_diameter), 2)));
  // given the channel number, i, it computes the wetted perimeter of
  // the subchannel based on the subchannel type: CENTER, EDGE or CORNER.
  auto i = _mesh.getSubchannelIndexFromPoint(p);
  auto subch_type = _mesh.getSubchannelType(i);

  if (subch_type == EChannelType::CENTER)
  {
    return 0.5 * rod_circumference + 0.5 * wire_circumference / cos(teta);
  }
  else if (subch_type == EChannelType::EDGE)
  {
    return 0.5 * rod_circumference + 0.5 * wire_circumference / cos(teta) + pitch;
  }
  else
  {
    return (rod_circumference + wire_circumference / cos(teta)) / 6.0 +
           2.0 / std::sqrt(3.0) * (rod_diameter / 2.0 + gap);
  }
}
