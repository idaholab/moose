#include "WettedPerimIC.h"
#include "SubChannelMesh.h"

registerMooseObject("SubChannelApp", WettedPerimIC);

InputParameters
WettedPerimIC::validParams()
{
  return SubChannelBaseIC::validParams();
}

WettedPerimIC::WettedPerimIC(const InputParameters & params) : SubChannelBaseIC(params) {}

Real
WettedPerimIC::value(const Point & p)
{
  // Define geometry parameters.
  auto pitch = _mesh.getPitch();
  auto rod_diameter = _mesh.getRodDiameter();
  auto gap = _mesh.getGap();
  auto rod_circumference = M_PI * rod_diameter;

  // Determine which channel this point is in and if that channel lies at an
  // edge or corner of the assembly.
  auto i = _mesh.getSubchannelIndexFromPoint(p);
  auto subch_type = _mesh.getSubchannelType(i);
  // Compute and return the wetted perimeter.
  if (subch_type == EChannelType::CORNER)
    return 0.25 * rod_circumference + pitch + 2 * gap;
  else if (subch_type == EChannelType::EDGE)
    return 0.5 * rod_circumference + pitch;
  else
    return rod_circumference;
}
