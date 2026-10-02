//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlowBoundary1PhaseAC.h"
#include "FlowChannel1PhaseAC.h"
#include "FlowChannel1PhasePhysics.h"
#include "THMNames.h"
#include "THMUtils.h"

InputParameters
FlowBoundary1PhaseAC::validParams()
{
  InputParameters params = ActionComponent::validParams();
  params.addRequiredParam<BoundaryName>(
      "input",
      "Name of the flow channel boundary to connect to, in the form 'component_name:in' or "
      "'component_name:out'");
  return params;
}

FlowBoundary1PhaseAC::FlowBoundary1PhaseAC(const InputParameters & params)
  : ActionComponent(params),
    _input(getParam<BoundaryName>("input")),
    _connected_component_name(THM::parseConnectedComponentName(_input)),
    _normal(THM::parseConnectionNormal(_input)),
    _boundary_uo_name(genName(name(), "boundary_uo"))
{
  addRequiredTask("add_user_object");
  addRequiredTask("add_bc");
}

const FlowChannel1PhasePhysics &
FlowBoundary1PhaseAC::connectedPhysics() const
{
  return _awh.getAction<FlowChannel1PhaseAC>(_connected_component_name).getPhysics();
}

void
FlowBoundary1PhaseAC::addWeakBCs()
{
  const std::string class_name = "ADBoundaryFlux3EqnBC";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<BoundaryName>>("boundary") = {_input};
  params.set<Real>("normal") = _normal;
  params.set<UserObjectName>("boundary_flux") = _boundary_uo_name;
  params.set<std::vector<VariableName>>("A_linear") = {THM::AREA_LINEAR};
  params.set<std::vector<VariableName>>("rhoA") = {THM::RHOA};
  params.set<std::vector<VariableName>>("rhouA") = {THM::RHOUA};
  params.set<std::vector<VariableName>>("rhoEA") = {THM::RHOEA};
  params.set<std::vector<VariableName>>("passives_times_area") = {};
  params.set<bool>("implicit") = THM::implicitTimeIntegrationFlag(_app);

  for (const auto & var : {THM::RHOA, THM::RHOUA, THM::RHOEA})
  {
    params.set<NonlinearVariableName>("variable") = var;
    getProblem().addBoundaryCondition(class_name, genName(name(), var, "bnd_flux_3eqn_bc"), params);
  }
}

void
FlowBoundary1PhaseAC::actOnAdditionalTasks()
{
  if (_current_task == "add_bc")
    addWeakBCs();
}
