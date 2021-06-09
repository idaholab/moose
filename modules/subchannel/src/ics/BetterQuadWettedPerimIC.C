#include "BetterQuadWettedPerimIC.h"
#include "BetterQuadSubChannelMesh.h"

registerMooseObject("SubChannelApp", BetterQuadWettedPerimIC);

InputParameters
BetterQuadWettedPerimIC::validParams()
{
  return BetterQuadSubChannelBaseIC::validParams();
}

BetterQuadWettedPerimIC::BetterQuadWettedPerimIC(const InputParameters & params)
  : BetterQuadSubChannelBaseIC(params)
{
}

Real
BetterQuadWettedPerimIC::value(const Point & p)
{
  auto pitch = _mesh.getPitch();
  auto rod_diameter = _mesh.getRodDiameter();
  auto gap = _mesh.getGap();
  auto rod_circumference = M_PI * rod_diameter;
  auto i = _mesh.getSubchannelIndexFromPoint(p);
  auto subch_type = _mesh.getSubchannelType(i);

  if (subch_type == EChannelType::CORNER)
    return 0.25 * rod_circumference + pitch + 2 * gap;
  else if (subch_type == EChannelType::EDGE)
    return 0.5 * rod_circumference + pitch;
  else
    return rod_circumference;
}
