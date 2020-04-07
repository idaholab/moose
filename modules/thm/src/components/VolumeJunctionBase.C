#include "VolumeJunctionBase.h"

InputParameters
VolumeJunctionBase::validParams()
{
  InputParameters params = FlowJunction::validParams();

  params.addRequiredParam<Real>("volume", "Volume of the junction [m^3]");
  params.addRequiredParam<Point>("position", "Spatial position of the center of the junction [m]");

  return params;
}

VolumeJunctionBase::VolumeJunctionBase(const InputParameters & params)
  : FlowJunction(params), _volume(getParam<Real>("volume")), _position(getParam<Point>("position"))
{
}
