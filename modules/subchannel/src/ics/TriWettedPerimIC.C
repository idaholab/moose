#include "TriWettedPerimIC.h"
#include "TriSubChannelMesh.h"

registerMooseObject("SubChannelApp", TriWettedPerimIC);

InputParameters
TriWettedPerimIC::validParams()
{
  return TriSubChannelBaseIC::validParams();
}

TriWettedPerimIC::TriWettedPerimIC(const InputParameters & params)
  : TriSubChannelBaseIC(params), _mesh(dynamic_cast<TriSubChannelMesh &>(_fe_problem.mesh()))
{
}

Real
TriWettedPerimIC::value(const Point & p)
{
  // Define geometry parameters.
  auto pitch = _mesh._pitch;
  auto rod_diameter = _mesh._rod_diameter;
  auto wire_diameter = pitch - rod_diameter;
  auto rod_circumference = libMesh::pi * rod_diameter;
  auto wire_circumference = libMesh::pi * wire_diameter;
  auto gap = _mesh._duct_to_rod_gap;

  // given the channel number, i, it computes the wetted perimeter of
  // the subchannel based on the subchannel type: CENTER, EDGE or CORNER.
  auto i = index_point(p);

  if (_mesh._subch_type[i] == TriSubChannelMesh::CENTER)
  {
    return 0.5 * rod_circumference + 0.5 * wire_circumference;
  }
  else if (_mesh._subch_type[i] == TriSubChannelMesh::EDGE)
  {
    return 0.5 * rod_circumference + 0.5 * wire_circumference + pitch;
  }
  else
  {
    return (rod_circumference + wire_circumference) / 6.0 +
           2.0 / std::sqrt(3.0) * (rod_diameter / 2.0 + gap);
  }
}
