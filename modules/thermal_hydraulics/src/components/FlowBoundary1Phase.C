//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlowBoundary1Phase.h"
#include "FlowChannel1Phase.h"
#include "FlowModelSinglePhase.h"

InputParameters
FlowBoundary1Phase::validParams()
{
  InputParameters params = FlowBoundary::validParams();
  return params;
}

FlowBoundary1Phase::FlowBoundary1Phase(const InputParameters & params)
  : FlowBoundary(params), _boundary_uo_name(genName(name(), "boundary_uo"))
{
}

void
FlowBoundary1Phase::init()
{
  FlowBoundary::init();

  if (hasComponentByName<FlowChannel1Phase>(_connected_component_name))
  {
    const FlowChannel1Phase & comp =
        getTHMProblem().getComponentByName<FlowChannel1Phase>(_connected_component_name);

    _numerical_flux_name = comp.getNumericalFluxUserObjectName();
  }
}

void
FlowBoundary1Phase::check() const
{
  FlowBoundary::check();

  checkComponentOfTypeExistsByName<FlowChannel1Phase>(_connected_component_name);
}

void
FlowBoundary1Phase::addWeakBC3Eqn()
{
  const std::string class_name = "ADBoundaryFlux3EqnBC";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
  params.set<Real>("normal") = _normal;
  params.set<UserObjectName>("boundary_flux") = _boundary_uo_name;
  params.set<std::vector<VariableName>>("A_linear") = {FlowModel::AREA_LINEAR};
  params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
  params.set<std::vector<VariableName>>("rhouA") = {FlowModelSinglePhase::RHOUA};
  params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
  params.set<bool>("implicit") = getTHMProblem().getImplicitTimeIntegrationFlag();

  const std::vector<NonlinearVariableName> variables{
      FlowModelSinglePhase::RHOA, FlowModelSinglePhase::RHOUA, FlowModelSinglePhase::RHOEA};

  for (const auto & var : variables)
  {
    params.set<NonlinearVariableName>("variable") = var;
    getTHMProblem().addBoundaryCondition(
        class_name, genName(name(), var, "bnd_flux_3eqn_bc"), params);
  }
}
