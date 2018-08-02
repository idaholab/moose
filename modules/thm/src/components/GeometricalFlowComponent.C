#include "GeometricalFlowComponent.h"
#include "FluidProperties.h"
#include "TwoPhaseNCGFluidProperties.h"

template <>
InputParameters
validParams<GeometricalFlowComponent>()
{
  InputParameters params = validParams<GeometricalComponent>();

  params.addClassDescription("Base class for geometrical components that have fluid flow");

  params.addRequiredParam<UserObjectName>("fp", "Name of fluid properties user object");
  params.addParam<MooseEnum>("rdg_slope_reconstruction",
                             FlowModel::getSlopeReconstructionMooseEnum("None"),
                             "Slope reconstruction type for rDG spatial discretization");
  params.addParam<bool>("implicit_rdg", true, "Use implicit time integration for rDG");

  return params;
}

GeometricalFlowComponent::GeometricalFlowComponent(const InputParameters & parameters)
  : GeometricalComponent(parameters),
    _fp_name(getParam<UserObjectName>("fp")),
    _n_ncgs(0),
    _rdg_flux_name(genName(name(), "rdg_flux")),
    _rdg_slope_reconstruction(
        getEnumParam<FlowModel::ESlopeReconstructionType>("rdg_slope_reconstruction")),
    _implicit_rdg(getParam<bool>("implicit_rdg"))
{
  if (_spatial_discretization == FlowModel::rDG)
    checkRDGRequiredParameter("implicit_rdg");
}

void
GeometricalFlowComponent::init()
{
  GeometricalComponent::init();

  _model_id = _app.getFlowModelID(_sim.getUserObject<FluidProperties>(_fp_name));

  if (_model_id == RELAP7::FM_TWO_PHASE_NCG)
  {
    const TwoPhaseNCGFluidProperties & fp =
        _sim.getUserObject<TwoPhaseNCGFluidProperties>(_fp_name);
    _n_ncgs = fp.getNumberOfNCGs();
  }
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
    mooseError(name(), ": Invalid pipe end type (", end_type, ").");
}

const RELAP7::FlowModelID &
GeometricalFlowComponent::getFlowModelID() const
{
  checkSetupStatus(INITIALIZED_PRIMARY);

  return _model_id;
}
