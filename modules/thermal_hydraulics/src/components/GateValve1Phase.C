//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GateValve1Phase.h"
#include "FlowChannel1Phase.h"
#include "FlowModelSinglePhase.h"

registerMooseObject("ThermalHydraulicsApp", GateValve1Phase);

InputParameters
GateValve1Phase::validParams()
{
  InputParameters params = FlowJunction1Phase::validParams();

  params.addRequiredParam<Real>("open_area_fraction", "Fraction of flow area that is open [-]");

  params.declareControllable("open_area_fraction");

  params.addClassDescription("Gate valve component for 1-phase flow");

  return params;
}

GateValve1Phase::GateValve1Phase(const InputParameters & params) : FlowJunction1Phase(params) {}

void
GateValve1Phase::setupMesh()
{
  FlowJunction1Phase::setupMesh();

  const auto & connected_elems = getConnectedElementIDs();
  if (connected_elems.size() == 2)
    getTHMProblem().augmentSparsity(connected_elems[0], connected_elems[1]);
}

void
GateValve1Phase::check() const
{
  FlowJunction1Phase::check();

  // Check that there are exactly 2 connections
  checkNumberOfConnections(2);

  // Log warning if slope reconstruction is used on one or more of the adjacent flow channels
  bool slope_reconstruction_used = false;
  for (const auto & connection : getConnections())
  {
    const std::string & comp_name = connection._component_name;
    if (hasComponentByName<FlowChannel1Phase>(comp_name))
    {
      const FlowChannel1Phase & comp = getComponentByName<FlowChannel1Phase>(comp_name);
      slope_reconstruction_used =
          slope_reconstruction_used || (comp.getSlopeReconstruction() != "none");
    }
  }
  if (slope_reconstruction_used)
    logWarning("Currently GateValve1Phase cannot perform slope reconstruction across the "
               "junction, so the slopes on the adjacent elements will be zero.");
}

void
GateValve1Phase::addMooseObjects()
{
  ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
  execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  // Add user object for computing and storing the fluxes
  {
    const std::string class_name = "ADGateValve1PhaseUserObject";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<BoundaryName>>("boundary") = _boundary_names;
    params.set<std::vector<Real>>("normals") = _normals;
    params.set<std::vector<processor_id_type>>("processor_ids") = getConnectedProcessorIDs();
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
    getTHMProblem().addUserObject(class_name, _junction_uo_name, params);

    connectObject(params, _junction_uo_name, "open_area_fraction");
  }

  const std::vector<NonlinearVariableName> var_names = {
      FlowModelSinglePhase::RHOA, FlowModelSinglePhase::RHOUA, FlowModelSinglePhase::RHOEA};

  // Add BC to each of the connected flow channels
  for (std::size_t i = 0; i < _boundary_names.size(); i++)
    for (std::size_t j = 0; j < var_names.size(); j++)
    {
      const std::string class_name = "ADGateValve1PhaseBC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<std::vector<BoundaryName>>("boundary") = {_boundary_names[i]};
      params.set<Real>("normal") = _normals[i];
      params.set<NonlinearVariableName>("variable") = var_names[j];
      params.set<UserObjectName>("gate_valve_uo") = _junction_uo_name;
      params.set<unsigned int>("connection_index") = i;
      params.set<std::vector<VariableName>>("A_elem") = {FlowModel::AREA};
      params.set<std::vector<VariableName>>("A_linear") = {FlowModel::AREA_LINEAR};
      params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
      params.set<std::vector<VariableName>>("rhouA") = {FlowModelSinglePhase::RHOUA};
      params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
      params.set<bool>("implicit") = getTHMProblem().getImplicitTimeIntegrationFlag();
      getTHMProblem().addBoundaryCondition(
          class_name, genName(name(), i, var_names[j] + ":" + class_name), params);
    }
}
