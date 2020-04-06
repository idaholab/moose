#include "JunctionOneToOne.h"
#include "GeometricalFlowComponent.h"
#include "FlowModelSinglePhase.h"
#include "FlowModelTwoPhase.h"
#include "FlowModelTwoPhaseNCG.h"
#include "THMMesh.h"

registerMooseObject("THMApp", JunctionOneToOne);

InputParameters
JunctionOneToOne::validParams()
{
  InputParameters params = FlowJunction::validParams();

  params.addClassDescription("Junction connecting one flow channel to one other flow channel");

  return params;
}

JunctionOneToOne::JunctionOneToOne(const InputParameters & params) : FlowJunction(params) {}

void
JunctionOneToOne::setupMesh()
{
  FlowJunction::setupMesh();

  std::vector<dof_id_type> connected_elems;
  const std::map<dof_id_type, std::vector<dof_id_type>> & node_to_elem = _mesh.nodeToElemMap();
  for (auto & nid : _nodes)
  {
    const auto & it = node_to_elem.find(nid);
    if (it == node_to_elem.end())
      mooseError(name(), ": failed to find node ", nid, "in the mesh!");

    const std::vector<dof_id_type> & elems = it->second;
    for (const auto & e : elems)
      connected_elems.push_back(e);
  }

  if (connected_elems.size() == 2)
    _sim.augmentSparsity(connected_elems[0], connected_elems[1]);
}

void
JunctionOneToOne::check() const
{
  FlowJunction::check();

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
    logWarning("Currently JunctionOneToOne cannot perform slope reconstruction across the "
               "junction, so the slopes on the adjacent elements will be zero.");
}

void
JunctionOneToOne::addMooseObjects()
{
  if (_flow_model_id == THM::FM_SINGLE_PHASE)
    addMooseObjects1Phase();
  else if (_flow_model_id == THM::FM_TWO_PHASE || _flow_model_id == THM::FM_TWO_PHASE_NCG)
    addMooseObjects2Phase();
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
JunctionOneToOne::addMooseObjects2Phase() const
{
  ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
  execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  std::vector<VariableName> ncg_vars;
  if (_flow_model_id == THM::FM_TWO_PHASE_NCG)
  {
    const FlowModelTwoPhaseNCG & fm = dynamic_cast<const FlowModelTwoPhaseNCG &>(*_flow_model);
    ncg_vars = fm.getNCGSolutionVars();
  }

  // Add user object for computing and storing the fluxes
  const std::string junction_uo_name = genName(name(), "junction_uo");
  {
    const std::string class_name = "JunctionOneToOne2PhaseUserObject";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<BoundaryName>>("boundary") = _boundary_names;
    params.set<std::vector<Real>>("normals") = _normals;
    // It is assumed that each channel should have the same numerical flux, so
    // just use the first one.
    params.set<UserObjectName>("numerical_flux") = _numerical_flux_names[0];
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    params.set<std::vector<VariableName>>("beta") = {FlowModelTwoPhase::BETA};
    params.set<std::vector<VariableName>>("arhoA_liquid") = {FlowModelTwoPhase::ALPHA_RHO_A_LIQUID};
    params.set<std::vector<VariableName>>("arhouA_liquid") = {
        FlowModelTwoPhase::ALPHA_RHOU_A_LIQUID};
    params.set<std::vector<VariableName>>("arhoEA_liquid") = {
        FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID};
    params.set<std::vector<VariableName>>("arhoA_vapor") = {FlowModelTwoPhase::ALPHA_RHO_A_VAPOR};
    params.set<std::vector<VariableName>>("arhouA_vapor") = {FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR};
    params.set<std::vector<VariableName>>("arhoEA_vapor") = {FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR};
    if (ncg_vars.size() > 0)
      params.set<std::vector<VariableName>>("aXrhoA_vapor") = ncg_vars;
    params.set<std::string>("junction_name") = name();
    params.set<ExecFlagEnum>("execute_on") = execute_on;
    _sim.addUserObject(class_name, junction_uo_name, params);
  }

  std::vector<NonlinearVariableName> var_names{FlowModelTwoPhase::BETA,
                                               FlowModelTwoPhase::ALPHA_RHO_A_LIQUID,
                                               FlowModelTwoPhase::ALPHA_RHOU_A_LIQUID,
                                               FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID,
                                               FlowModelTwoPhase::ALPHA_RHO_A_VAPOR,
                                               FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR,
                                               FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR};
  var_names.reserve(var_names.size() + ncg_vars.size());
  var_names.insert(var_names.end(), ncg_vars.begin(), ncg_vars.end());

  // Add BC to each of the connected flow channels
  for (std::size_t i = 0; i < _boundary_names.size(); i++)
    for (std::size_t j = 0; j < var_names.size(); j++)
    {
      const std::string class_name = "JunctionOneToOne2PhaseBC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<std::vector<BoundaryName>>("boundary") = {_boundary_names[i]};
      params.set<Real>("normal") = _normals[i];
      params.set<NonlinearVariableName>("variable") = var_names[j];
      params.set<UserObjectName>("junction_uo") = junction_uo_name;
      params.set<unsigned int>("connection_index") = i;
      params.set<std::vector<VariableName>>("A_elem") = {FlowModel::AREA};
      params.set<std::vector<VariableName>>("A_linear") = {FlowModel::AREA_LINEAR};
      params.set<std::vector<VariableName>>("beta") = {FlowModelTwoPhase::BETA};
      params.set<std::vector<VariableName>>("arhoA_liquid") = {
          FlowModelTwoPhase::ALPHA_RHO_A_LIQUID};
      params.set<std::vector<VariableName>>("arhouA_liquid") = {
          FlowModelTwoPhase::ALPHA_RHOU_A_LIQUID};
      params.set<std::vector<VariableName>>("arhoEA_liquid") = {
          FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID};
      params.set<std::vector<VariableName>>("arhoA_vapor") = {FlowModelTwoPhase::ALPHA_RHO_A_VAPOR};
      params.set<std::vector<VariableName>>("arhouA_vapor") = {
          FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR};
      params.set<std::vector<VariableName>>("arhoEA_vapor") = {
          FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR};
      if (ncg_vars.size() > 0)
        params.set<std::vector<VariableName>>("aXrhoA_vapor") = ncg_vars;
      params.set<bool>("implicit") = _sim.getImplicitTimeIntegrationFlag();
      _sim.addBoundaryCondition(
          class_name, genName(name(), i, var_names[j] + ":" + class_name), params);
    }
}
