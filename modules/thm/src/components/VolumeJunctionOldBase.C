#include "VolumeJunctionOldBase.h"
#include "GeometricalFlowComponent.h"

template <>
InputParameters
validParams<VolumeJunctionOldBase>()
{
  InputParameters params = validParams<JunctionWithLossesBase>();

  params.addParam<Real>("initial_p", 1e5, "Initial pressure of this junction");
  params.addParam<Real>("initial_T", 300, "Initial temperature of this junction");
  params.addParam<Real>("initial_vel", 0, "Initial average speed of this junction");
  params.addParam<Real>(
      "initial_alpha_vapor", 1., "Initial vapor volume fraction of this junction");

  params.addRequiredParam<Point>("center", "geometric center of the volume");
  params.addRequiredParam<Real>("volume", "Volume of the component");

  std::vector<Real> sf(3, 1.e-3);
  params.addParam<std::vector<Real>>("scale_factors", sf, "variable scale factor");

  return params;
}

VolumeJunctionOldBase::VolumeJunctionOldBase(const InputParameters & parameters)
  : JunctionWithLossesBase(parameters),
    _initial_p(getParam<Real>("initial_p")),
    _initial_vel(getParam<Real>("initial_vel")),
    _initial_T(getParam<Real>("initial_T")),
    _initial_void_fraction(getParam<Real>("initial_alpha_vapor")),
    _center(getParam<Point>("center")),
    _volume(getParam<Real>("volume")),
    _scale_factors(getParam<std::vector<Real>>("scale_factors"))
{
}

void
VolumeJunctionOldBase::computeDeltaH(Real H_junction)
{
  for (const auto & connection : getConnections())
  {
    const GeometricalFlowComponent & gc =
        _sim.getComponentByName<GeometricalFlowComponent>(connection._geometrical_component_name);
    for (const auto & gc_connection : gc.getConnections(connection._end_type))
      _deltaH.push_back(H_junction - gc_connection._position(2));
  }
}
