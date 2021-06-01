#include "TriFlowAreaIC.h"
#include "TriSubChannelMesh.h"

registerMooseObject("SubChannelApp", TriFlowAreaIC);

InputParameters
TriFlowAreaIC::validParams()
{
  InputParameters params = TriSubChannelBaseIC::validParams();
  return params;
}

TriFlowAreaIC::TriFlowAreaIC(const InputParameters & params) : TriSubChannelBaseIC(params) {}

Real
TriFlowAreaIC::value(const Point & p)
{
  auto pitch = _mesh.getPitch();
  auto rod_diameter = _mesh.getRodDiameter();
  auto wire_diameter = _mesh.getWireDiameter();
  auto wire_lead_length = _mesh.getWireLeadLength();
  auto gap = _mesh.getDuctToRodGap();
  auto theta = std::acos(wire_lead_length /
                         std::sqrt(std::pow(wire_lead_length, 2) +
                                   std::pow(libMesh::pi * (rod_diameter + wire_diameter), 2)));
  auto i = _mesh.getSubchannelIndexFromPoint(p);
  // given the channel number, i, it computes the flow area of the subchannel
  // based on the subchannel type: CENTER, EDGE or CORNER.
  auto subch_type = _mesh.getSubchannelType(i);
  if (subch_type == EChannelType::CENTER)
  {
    return std::pow(pitch, 2) * std::sqrt(3.0) / 4.0 -
           libMesh::pi * std::pow(rod_diameter, 2.0) / 8.0 -
           libMesh::pi * std::pow(wire_diameter, 2) / 8.0 / std::cos(theta);
  }
  else if (subch_type == EChannelType::EDGE)
  {
    return pitch * (rod_diameter / 2.0 + gap) - libMesh::pi * std::pow(rod_diameter, 2.0) / 8.0 -
           libMesh::pi * std::pow(wire_diameter, 2.0) / 8.0 / std::cos(theta);
  }
  else
  {
    return 1.0 / std::sqrt(3.0) * std::pow((rod_diameter / 2.0 + gap), 2.0) -
           libMesh::pi * std::pow(rod_diameter, 2.0) / 24.0 -
           libMesh::pi / 24.0 * std::pow(wire_diameter, 2.0) / std::cos(theta);
  }
}
