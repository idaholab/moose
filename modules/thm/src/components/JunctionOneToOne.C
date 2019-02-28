#include "JunctionOneToOne.h"
#include "GeometricalFlowComponent.h"
#include "FlowModelSinglePhase.h"

registerMooseObject("THMApp", JunctionOneToOne);

template <>
InputParameters
validParams<JunctionOneToOne>()
{
  InputParameters params = validParams<FlowJunction>();

  params.addClassDescription("Junction connecting one flow channel to one other flow channel");

  return params;
}

JunctionOneToOne::JunctionOneToOne(const InputParameters & params) : FlowJunction(params) {}

void
JunctionOneToOne::check() const
{
  FlowJunction::check();

  if (_flow_model_id != THM::FM_SINGLE_PHASE)
    logModelNotImplementedError(_flow_model_id);

  // Check that there are exactly 2 connections
  checkNumberOfConnections(2);

  // Log warning if slope reconstruction is used on one or more of the adjacent flow channels
  bool slope_reconstruction_used = false;
  for (const auto & connection : getConnections())
  {
    const std::string & gc_name = connection._geometrical_component_name;
    if (hasComponentByName<GeometricalFlowComponent>(gc_name))
    {
      const GeometricalFlowComponent & gc = getComponentByName<GeometricalFlowComponent>(gc_name);
      slope_reconstruction_used =
          slope_reconstruction_used || (gc.getSlopeReconstruction() != "none");
    }
  }
  if (slope_reconstruction_used)
    logWarning("Currently JunctionOneToOne cannot perform slope reconstruction across the "
               "junction, so the slopes on the adjacent elements will be zero.");
}

void
JunctionOneToOne::addMooseObjects()
{
  if (_flow_model_id == THM::FM_SINGLE_PHASE)
    addMooseObjects1Phase();
  else
    mooseError(name() + ": Not implemented.");
}

void
JunctionOneToOne::addMooseObjects1Phase() const
{
  ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
  execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  // Add user object for computing and storing the fluxes
  const std::string junction_uo_name = genName(name(), "junction_uo");
  {
    const std::string class_name = "JunctionOneToOne1PhaseUserObject";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<BoundaryName>>("boundary") = _boundary_names;
    params.set<std::vector<Real>>("normals") = _normals;
    // It is assumed that each channel should have the same numerical flux, so
    // just use the first one.
    params.set<UserObjectName>("numerical_flux") = _numerical_flux_names[0];
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {FlowModelSinglePhase::RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
    params.set<std::string>("junction_name") = name();
    params.set<ExecFlagEnum>("execute_on") = execute_on;
    _sim.addUserObject(class_name, junction_uo_name, params);
  }

  const std::vector<NonlinearVariableName> var_names = {
      FlowModelSinglePhase::RHOA, FlowModelSinglePhase::RHOUA, FlowModelSinglePhase::RHOEA};

  // Add BC to each of the connected flow channels
  for (std::size_t i = 0; i < _boundary_names.size(); i++)
    for (std::size_t j = 0; j < var_names.size(); j++)
    {
      const std::string class_name = "JunctionOneToOne1PhaseBC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<std::vector<BoundaryName>>("boundary") = {_boundary_names[i]};
      params.set<Real>("normal") = _normals[i];
      params.set<NonlinearVariableName>("variable") = var_names[j];
      params.set<UserObjectName>("junction_uo") = junction_uo_name;
      params.set<unsigned int>("connection_index") = i;
      params.set<std::vector<VariableName>>("A_elem") = {FlowModel::AREA};
      params.set<std::vector<VariableName>>("A_linear") = {_A_linear_names[i]};
      params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
      params.set<std::vector<VariableName>>("rhouA") = {FlowModelSinglePhase::RHOUA};
      params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
      params.set<bool>("implicit") = _sim.getImplicitTimeIntegrationFlag();
      _sim.addBoundaryCondition(
          class_name, genName(name(), i, var_names[j] + ":" + class_name), params);
    }
}
