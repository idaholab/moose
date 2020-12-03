#include "TriFlowAreaIC.h"
#include "TriSubChannelMesh.h"

registerMooseObject("SubChannelApp", TriFlowAreaIC);

InputParameters
TriFlowAreaIC::validParams()
{
  InputParameters params = TriSubChannelBaseIC::validParams();
  return params;
}

TriFlowAreaIC::TriFlowAreaIC(const InputParameters & params)
  : TriSubChannelBaseIC(params), _mesh(dynamic_cast<TriSubChannelMesh &>(_fe_problem.mesh()))
{
}

Real
TriFlowAreaIC::value(const Point & p)
{
  auto pitch = _mesh._pitch;
  auto rod_diameter = _mesh._rod_diameter;
  auto wire_diameter = pitch - rod_diameter;
  auto gap = _mesh._duct_to_rod_gap;

  auto i = index_point(p);
  // given the channel number, i, it computes the flow area of the subchannel
  // based on the subchannel type: CENTER, EDGE or CORNER.
  if (_mesh._subch_type[i] == TriSubChannelMesh::CENTER)
  {
    return std::pow(pitch, 2) * std::sqrt(3.0) / 4.0 -
           libMesh::pi * std::pow(rod_diameter, 2.0) / 8.0 -
           libMesh::pi * std::pow(wire_diameter, 2) / 8.0;
  }
  else if (_mesh._subch_type[i] == TriSubChannelMesh::EDGE)
  {
    return pitch * (rod_diameter / 2.0 + gap) - libMesh::pi * std::pow(rod_diameter, 2.0) / 8.0 -
           libMesh::pi * std::pow(wire_diameter, 2.0) / 8.0;
  }
  else
  {
    return 1.0 / std::sqrt(3.0) * std::pow((rod_diameter / 2.0 + gap), 2.0) -
           libMesh::pi * std::pow(rod_diameter, 2.0) / 24.0 -
           libMesh::pi / 24.0 * std::pow(wire_diameter, 2.0);
  }
}
