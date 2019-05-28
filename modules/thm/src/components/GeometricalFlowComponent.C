#include "GeometricalFlowComponent.h"
#include "FluidProperties.h"
#include "SlopeReconstruction1DInterface.h"

template <>
InputParameters
validParams<GeometricalFlowComponent>()
{
  InputParameters params = validParams<GeometricalComponent>();

  params.addClassDescription("Base class for geometrical components that have fluid flow");

  params.addRequiredParam<UserObjectName>("fp", "Name of fluid properties user object");
  params.addParam<MooseEnum>(
      "rdg_slope_reconstruction",
      SlopeReconstruction1DInterface::getSlopeReconstructionMooseEnum("None"),
      "Slope reconstruction type for rDG spatial discretization");

  return params;
}

GeometricalFlowComponent::GeometricalFlowComponent(const InputParameters & parameters)
  : GeometricalComponent(parameters),
    _spatial_discretization(_sim.getSpatialDiscretization()),
    _fp_name(getParam<UserObjectName>("fp")),
    _A_linear_name(_spatial_discretization == FlowModel::rDG ? FlowModel::AREA + "_linear"
                                                             : FlowModel::AREA),
    _numerical_flux_name(genName(name(), "numerical_flux")),
    _rdg_int_var_uo_name(genName(name(), "rdg_int_var_uo")),
    _rdg_slope_reconstruction(getParam<MooseEnum>("rdg_slope_reconstruction"))
{
}

void
GeometricalFlowComponent::init()
{
  GeometricalComponent::init();

  _model_id = _app.getFlowModelID(_sim.getUserObject<FluidProperties>(_fp_name));
}

bool
GeometricalFlowComponent::usingSecondOrderMesh() const
{
  if (_spatial_discretization == FlowModel::rDG)
    return false;
  else
    return _sim.getFlowFEType().order == SECOND;
}

const std::vector<GeometricalFlowComponent::Connection> &
GeometricalFlowComponent::getConnections(FlowConnection::EEndType end_type) const
{
  checkSetupStatus(MESH_PREPARED);

  std::map<FlowConnection::EEndType, std::vector<Connection>>::const_iterator it =
      _connections.find(end_type);
  if (it != _connections.end())
    return it->second;
  else
    mooseError(name(), ": Invalid flow channel end type (", end_type, ").");
}

const THM::FlowModelID &
GeometricalFlowComponent::getFlowModelID() const
{
  checkSetupStatus(INITIALIZED_PRIMARY);

  return _model_id;
}
