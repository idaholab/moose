#include "QuadInterWrapperWettedPerimIC.h"
#include "QuadInterWrapperMesh.h"

registerMooseObject("SubChannelApp", QuadInterWrapperWettedPerimIC);

InputParameters
QuadInterWrapperWettedPerimIC::validParams()
{
  return QuadSubChannelBaseIC::validParams();
}

QuadInterWrapperWettedPerimIC::QuadInterWrapperWettedPerimIC(const InputParameters & params) : QuadInterWrapperBaseIC(params)
{
}

Real
QuadInterWrapperWettedPerimIC::value(const Point & p)
{
  auto pitch = _mesh.getPitch();
  auto side_x = _mesh.getSideX();
  auto side_y = _mesh.getSideY();
  auto gap = _mesh.getGap();
  auto square_perimeter = 2.0 * side_x + 2.0 * side_y;
  auto i = _mesh.getSubchannelIndexFromPoint(p);
  auto subch_type = _mesh.getSubchannelType(i);

  if (subch_type == EChannelType::CORNER)
    return 0.25 * square_perimeter + pitch + 2 * gap;
  else if (subch_type == EChannelType::EDGE)
    return 0.5 * square_perimeter + pitch;
  else
    return square_perimeter;
}
