#include "VolumeJunctionBase.h"

template <>
InputParameters
validParams<VolumeJunctionBase>()
{
  InputParameters params = validParams<FlowJunction>();

  params.addRequiredParam<Real>("volume", "Volume of the junction [m^3]");
  params.addRequiredParam<Point>("position", "Spatial position of the center of the junction");

  return params;
}

VolumeJunctionBase::VolumeJunctionBase(const InputParameters & params)
  : FlowJunction(params), _volume(getParam<Real>("volume")), _position(getParam<Point>("position"))
{
}
