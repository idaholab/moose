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
FlowBoundary1Phase::check() const
{
  FlowBoundary1PhaseBase::check();

  checkComponentOfTypeExistsByName<FlowChannel1Phase>(_connected_component_name);
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
  params.set<bool>("implicit") = getTHMProblem().getImplicitTimeIntegrationFlag();

  const std::vector<NonlinearVariableName> variables{THM::RHOA, THM::RHOUA, THM::RHOEA};

  for (const auto & var : variables)
  {
    params.set<NonlinearVariableName>("variable") = var;
    getTHMProblem().addBoundaryCondition(
        class_name, genName(name(), var, "bnd_flux_3eqn_bc"), params);
  }
}
