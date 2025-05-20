//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlowBoundaryGasMix.h"
#include "FlowChannelGasMix.h"
#include "THMNames.h"

InputParameters
FlowBoundaryGasMix::validParams()
{
  InputParameters params = FlowBoundary1PhaseBase::validParams();
  return params;
}

FlowBoundaryGasMix::FlowBoundaryGasMix(const InputParameters & params)
  : FlowBoundary1PhaseBase(params)
{
}

void
FlowBoundaryGasMix::check() const
{
  FlowBoundary1PhaseBase::check();

  checkComponentOfTypeExistsByName<FlowChannelGasMix>(_connected_component_name);
}

void
FlowBoundaryGasMix::addWeakBCs()
{
  const std::string class_name = "BoundaryFluxGasMixBC";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
  params.set<Real>("normal") = _normal;
  params.set<UserObjectName>("boundary_flux") = _boundary_uo_name;
  params.set<std::vector<VariableName>>("A_linear") = {THM::AREA_LINEAR};
  params.set<std::vector<VariableName>>("xirhoA") = {THM::XIRHOA};
  params.set<std::vector<VariableName>>("rhoA") = {THM::RHOA};
  params.set<std::vector<VariableName>>("rhouA") = {THM::RHOUA};
  params.set<std::vector<VariableName>>("rhoEA") = {THM::RHOEA};
  params.set<bool>("implicit") = getTHMProblem().getImplicitTimeIntegrationFlag();

  const std::vector<NonlinearVariableName> variables{
      THM::XIRHOA, THM::RHOA, THM::RHOUA, THM::RHOEA};

  for (const auto & var : variables)
  {
    params.set<NonlinearVariableName>("variable") = var;
    getTHMProblem().addBoundaryCondition(class_name, genName(name(), var, "bc"), params);
  }
}
