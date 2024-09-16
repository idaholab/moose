//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "JunctionOneToOne1Phase.h"
#include "FlowChannel1Phase.h"
#include "FlowModelSinglePhase.h"
#include "THMMesh.h"
#include "SlopeReconstruction1DInterface.h"

registerMooseObject("ThermalHydraulicsApp", JunctionOneToOne1Phase);

InputParameters
JunctionOneToOne1Phase::validParams()
{
  InputParameters params = FlowJunction1Phase::validParams();

  params.addClassDescription(
      "Junction connecting one flow channel to one other flow channel for 1-phase flow");

  return params;
}

JunctionOneToOne1Phase::JunctionOneToOne1Phase(const InputParameters & params)
  : FlowJunction1Phase(params),
    _slope_reconstruction(
        SlopeReconstruction1DInterface<true>::getSlopeReconstructionMooseEnum("None"))
{
}

void
JunctionOneToOne1Phase::setupMesh()
{
  FlowJunction1Phase::setupMesh();

  const auto & connected_elems = getConnectedElementIDs();
  if (connected_elems.size() == 2)
    getTHMProblem().augmentSparsity(connected_elems[0], connected_elems[1]);
}

void
JunctionOneToOne1Phase::init()
{
  FlowJunction1Phase::init();

  // Get slope reconstruction option used
  for (const auto & connection : getConnections())
  {
    const std::string & comp_name = connection._component_name;
    if (hasComponentByName<FlowChannel1Phase>(comp_name))
    {
      const FlowChannel1Phase & comp = getComponentByName<FlowChannel1Phase>(comp_name);
      _slope_reconstruction = comp.getSlopeReconstruction();
    }
  }
}

void
JunctionOneToOne1Phase::check() const
{
  FlowJunction1Phase::check();

  // Check that there are exactly 2 connections
  checkNumberOfConnections(2);
}

void
JunctionOneToOne1Phase::addMooseObjects()
{
  ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
  execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  // Add user object for computing and storing the fluxes
  {
    const std::string class_name = "ADJunctionOneToOne1PhaseUserObject";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<BoundaryName>>("boundary") = _boundary_names;
    params.set<std::vector<Real>>("normals") = _normals;
    params.set<std::vector<processor_id_type>>("processor_ids") = getConnectedProcessorIDs();
    params.set<UserObjectName>("fluid_properties") = _fp_name;
    // It is assumed that each channel should have the same numerical flux, so
    // just use the first one.
    params.set<UserObjectName>("numerical_flux") = _numerical_flux_names[0];
    params.set<std::vector<VariableName>>("A_elem") = {FlowModel::AREA};
    params.set<std::vector<VariableName>>("A_linear") = {FlowModel::AREA_LINEAR};
    params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {FlowModelSinglePhase::RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
    params.set<std::string>("junction_name") = name();
    params.set<MooseEnum>("scheme") = _slope_reconstruction;
    params.set<ExecFlagEnum>("execute_on") = execute_on;
    getTHMProblem().addUserObject(class_name, _junction_uo_name, params);
  }

  const std::vector<NonlinearVariableName> var_names = {
      FlowModelSinglePhase::RHOA, FlowModelSinglePhase::RHOUA, FlowModelSinglePhase::RHOEA};

  // Add BC to each of the connected flow channels
  for (std::size_t i = 0; i < _boundary_names.size(); i++)
    for (std::size_t j = 0; j < var_names.size(); j++)
    {
      const std::string class_name = "ADJunctionOneToOne1PhaseBC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<std::vector<BoundaryName>>("boundary") = {_boundary_names[i]};
      params.set<Real>("normal") = _normals[i];
      params.set<NonlinearVariableName>("variable") = var_names[j];
      params.set<UserObjectName>("junction_uo") = _junction_uo_name;
      params.set<unsigned int>("connection_index") = i;
      params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
      params.set<std::vector<VariableName>>("rhouA") = {FlowModelSinglePhase::RHOUA};
      params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
      params.set<bool>("implicit") = getTHMProblem().getImplicitTimeIntegrationFlag();
      getTHMProblem().addBoundaryCondition(
          class_name, genName(name(), i, var_names[j] + ":" + class_name), params);
    }
}
