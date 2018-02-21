#include "GeometricalFlowComponent.h"
#include "FluidProperties.h"

template <>
InputParameters
validParams<GeometricalFlowComponent>()
{
  InputParameters params = validParams<GeometricalComponent>();

  params.addClassDescription("Base class for geometrical components that have fluid flow");

  params.addRequiredParam<UserObjectName>("fp", "Name of fluid properties user object");

  return params;
}

GeometricalFlowComponent::GeometricalFlowComponent(const InputParameters & parameters)
  : GeometricalComponent(parameters), _fp_name(getParam<UserObjectName>("fp"))
{
}

void
GeometricalFlowComponent::init()
{
  GeometricalComponent::init();

  _model_id = _app.getFlowModelID(_sim.getUserObject<FluidProperties>(_fp_name));
}

const std::vector<GeometricalFlowComponent::Connection> &
GeometricalFlowComponent::getConnections(PipeConnectable::EEndType end_type) const
{
  checkSetupStatus(MESH_PREPARED);

  std::map<PipeConnectable::EEndType, std::vector<Connection>>::const_iterator it =
      _connections.find(end_type);
  if (it != _connections.end())
    return it->second;
  else
    mooseError(name(), ": Invalid pipe end type (", end_type, ").");
}

const RELAP7::FlowModelID &
GeometricalFlowComponent::getFlowModelID() const
{
  checkSetupStatus(INITIALIZED_PRIMARY);

  return _model_id;
}
