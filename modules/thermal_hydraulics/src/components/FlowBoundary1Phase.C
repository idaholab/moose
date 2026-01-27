//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlowBoundary1Phase.h"
#include "FlowChannel1Phase.h"
#include "FlowModelSinglePhase.h"
#include "THMNames.h"

InputParameters
FlowBoundary1Phase::validParams()
{
  InputParameters params = FlowBoundary1PhaseBase::validParams();
  return params;
}

FlowBoundary1Phase::FlowBoundary1Phase(const InputParameters & params)
  : FlowBoundary1PhaseBase(params)
{
}

void
FlowBoundary1Phase::init()
{
  FlowBoundary1PhaseBase::init();

  if (hasComponentByName<FlowChannel1Phase>(_connected_component_name))
  {
    auto flow_model_1phase = dynamic_cast<const FlowModelSinglePhase *>(_flow_model.get());
    mooseAssert(flow_model_1phase, "Incompatible flow model");
    _passives_times_area = flow_model_1phase->passiveTransportSolutionVariableNames();
  }
}

void
FlowBoundary1Phase::check() const
{
  FlowBoundary1PhaseBase::check();

  checkComponentOfTypeExistsByName<FlowChannel1Phase>(_connected_component_name);

  if (_passives_times_area.size() > 0 && !supportsPassiveTransport())
    logError("Passive transport has not been implemented for this Component type.");
}

void
FlowBoundary1Phase::addWeakBCs()
{
  const std::string class_name = "ADBoundaryFlux3EqnBC";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
  params.set<Real>("normal") = _normal;
  params.set<UserObjectName>("boundary_flux") = _boundary_uo_name;
  params.set<std::vector<VariableName>>("A_linear") = {THM::AREA_LINEAR};
  params.set<std::vector<VariableName>>("rhoA") = {THM::RHOA};
  params.set<std::vector<VariableName>>("rhouA") = {THM::RHOUA};
  params.set<std::vector<VariableName>>("rhoEA") = {THM::RHOEA};
  params.set<std::vector<VariableName>>("passives_times_area") = _passives_times_area;
  params.set<bool>("implicit") = getTHMProblem().getImplicitTimeIntegrationFlag();

  auto flow_model_1phase = dynamic_cast<const FlowModelSinglePhase *>(_flow_model.get());
  for (const auto & var : flow_model_1phase->solutionVariableNames())
  {
    params.set<NonlinearVariableName>("variable") = var;
    getTHMProblem().addBoundaryCondition(
        class_name, genName(name(), var, "bnd_flux_3eqn_bc"), params);
  }
}
