#include "BetterQuadFlowAreaIC.h"

registerMooseObject("SubChannelApp", BetterQuadFlowAreaIC);

InputParameters
BetterQuadFlowAreaIC::validParams()
{
  InputParameters params = BetterQuadSubChannelBaseIC::validParams();
  return params;
}

BetterQuadFlowAreaIC::BetterQuadFlowAreaIC(const InputParameters & params)
  : BetterQuadSubChannelBaseIC(params)
{
}

Real
BetterQuadFlowAreaIC::value(const Point & p)
{
  auto pitch = _mesh.getPitch();
  auto rod_diameter = _mesh.getRodDiameter();
  auto gap = _mesh.getGap();

  // Compute the flow area for a standard channel (one that touches 4 pins).
  auto rod_area = 0.25 * M_PI * rod_diameter * rod_diameter;
  auto standard_area = pitch * pitch - rod_area;

  auto i = _mesh.getSubchannelIndexFromPoint(p);
  auto subch_type = _mesh.getSubchannelType(i);
  if (subch_type == EChannelType::CORNER)
    return standard_area * 0.25 + pitch * gap + gap * gap;
  else if (subch_type == EChannelType::EDGE)
    return standard_area * 0.5 + pitch * gap;
  else
    return standard_area;
}
