#include "GateValve.h"
#include "GeometricalFlowComponent.h"
#include "FlowModelSinglePhase.h"
#include "FlowModelTwoPhase.h"

registerMooseObject("THMApp", GateValve);

template <>
InputParameters
validParams<GateValve>()
{
  InputParameters params = validParams<FlowJunction>();

  params.addRequiredParam<Real>("open_area_fraction",
                                "Fraction of possible flow area that is open");

  params.addClassDescription("Gate valve component");

  return params;
}

GateValve::GateValve(const InputParameters & params) : FlowJunction(params) {}

void
GateValve::check() const
{
  FlowJunction::check();

  if (_flow_model_id != THM::FM_SINGLE_PHASE)
    logModelNotImplementedError(_flow_model_id);

  if (_spatial_discretization != FlowModel::rDG)
    logSpatialDiscretizationNotImplementedError(_spatial_discretization);

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
    logWarning("Currently GateValve cannot perform slope reconstruction across the "
               "junction, so the slopes on the adjacent elements will be zero.");
}

void
GateValve::addMooseObjects()
{
  if (_flow_model_id == THM::FM_SINGLE_PHASE)
    addMooseObjects1Phase();
  else if (_flow_model_id == THM::FM_TWO_PHASE)
    addMooseObjects2Phase();
}

void
GateValve::addMooseObjects1Phase() const
{
  ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
  execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  // Add user object for computing and storing the fluxes
  const std::string uo_name = genName(name(), "uo");
  {
    const std::string class_name = "GateValve1PhaseUserObject";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<BoundaryName>>("boundary") = _boundary_names;
    params.set<std::vector<Real>>("normals") = _normals;
    // It is assumed that each channel should have the same numerical flux, so
    // just use the first one.
    params.set<UserObjectName>("numerical_flux") = _numerical_flux_names[0];
    params.set<Real>("open_area_fraction") = getParam<Real>("open_area_fraction");
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {FlowModelSinglePhase::RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
    params.set<std::string>("component_name") = name();
    params.set<ExecFlagEnum>("execute_on") = execute_on;
    _sim.addUserObject(class_name, uo_name, params);

    connectObject(params, uo_name, "open_area_fraction");
  }

  const std::vector<NonlinearVariableName> var_names = {
      FlowModelSinglePhase::RHOA, FlowModelSinglePhase::RHOUA, FlowModelSinglePhase::RHOEA};

  // Add BC to each of the connected flow channels
  for (std::size_t i = 0; i < _boundary_names.size(); i++)
    for (std::size_t j = 0; j < var_names.size(); j++)
    {
      const std::string class_name = "GateValve1PhaseBC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<std::vector<BoundaryName>>("boundary") = {_boundary_names[i]};
      params.set<Real>("normal") = _normals[i];
      params.set<NonlinearVariableName>("variable") = var_names[j];
      params.set<UserObjectName>("gate_valve_uo") = uo_name;
      params.set<unsigned int>("connection_index") = i;
      params.set<std::vector<VariableName>>("A_elem") = {FlowModel::AREA};
      params.set<std::vector<VariableName>>("A_linear") = {FlowModel::AREA_LINEAR};
      params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
      params.set<std::vector<VariableName>>("rhouA") = {FlowModelSinglePhase::RHOUA};
      params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
      params.set<bool>("implicit") = _sim.getImplicitTimeIntegrationFlag();
      _sim.addBoundaryCondition(
          class_name, genName(name(), i, var_names[j] + ":" + class_name), params);
    }
}

void
GateValve::addMooseObjects2Phase() const
{
}
