#include "GeometricalFlowComponent.h"
#include "FluidProperties.h"
#include "SlopeReconstruction1DInterface.h"

InputParameters
GeometricalFlowComponent::validParams()
{
  InputParameters params = GeometricalComponent::validParams();
  params += GravityInterface::validParams();

  params.addClassDescription("Base class for geometrical components that have fluid flow");

  params.addRequiredParam<UserObjectName>("fp", "Fluid properties user object");
  params.addParam<MooseEnum>(
      "rdg_slope_reconstruction",
      SlopeReconstruction1DInterface<true>::getSlopeReconstructionMooseEnum("None"),
      "Slope reconstruction type for rDG spatial discretization");

  return params;
}

GeometricalFlowComponent::GeometricalFlowComponent(const InputParameters & parameters)
  : GeometricalComponent(parameters),
    GravityInterface(parameters),
    _gravity_angle(MooseUtils::absoluteFuzzyEqual(_gravity_magnitude, 0.0)
                       ? 0.0
                       : std::acos(_dir * _gravity_vector / (_dir.norm() * _gravity_magnitude)) *
                             180 / M_PI),
    _fp_name(getParam<UserObjectName>("fp")),
    _numerical_flux_name(genName(name(), "numerical_flux")),
    _rdg_int_var_uo_name(genName(name(), "rdg_int_var_uo")),
    _rdg_slope_reconstruction(getParam<MooseEnum>("rdg_slope_reconstruction"))
{
}

bool
GeometricalFlowComponent::usingSecondOrderMesh() const
{
  return false;
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
