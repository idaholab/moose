#include "QuadInterWrapperFlowAreaIC.h"

registerMooseObject("SubChannelApp", QuadInterWrapperFlowAreaIC);

InputParameters
QuadInterWrapperFlowAreaIC::validParams()
{
  InputParameters params = QuadInterWrapperBaseIC::validParams();
  return params;
}

QuadInterWrapperFlowAreaIC::QuadInterWrapperFlowAreaIC(const InputParameters & params) : QuadInterWrapperBaseIC(params) {}

Real
QuadInterWrapperFlowAreaIC::value(const Point & p)
{
  auto assembly_pitch = _mesh.getPitch();
  auto side_x = _mesh.getSideX();
  auto side_y = _mesh.getSideY();
  auto gap = _mesh.getGap();

  // Compute the flow area for a standard inter-wrapper channel (one that touches 4 assemblies).
  auto assembly_area = side_x * side_y;
  auto standard_area = assembly_pitch * assembly_pitch - assembly_area;

  auto i = _mesh.getSubchannelIndexFromPoint(p);
  auto subch_type = _mesh.getSubchannelType(i);
  if (subch_type == EChannelType::CORNER)
    return standard_area * 0.25 + assembly_pitch * gap + gap * gap;
  else if (subch_type == EChannelType::EDGE)
    return standard_area * 0.5 + assembly_pitch * gap;
  else
    return standard_area;
}
