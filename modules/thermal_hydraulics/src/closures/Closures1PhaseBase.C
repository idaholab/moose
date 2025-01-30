//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Closures1PhaseBase.h"
#include "FlowModelSinglePhase.h"
#include "FlowChannel1Phase.h"

InputParameters
Closures1PhaseBase::validParams()
{
  InputParameters params = ClosuresBase::validParams();
  return params;
}

Closures1PhaseBase::Closures1PhaseBase(const InputParameters & params) : ClosuresBase(params) {}

void
Closures1PhaseBase::addWallFrictionFunctionMaterial(const FlowChannel1Phase & flow_channel) const
{
  const FunctionName & f_D_fn_name = flow_channel.getParam<FunctionName>("f");
  flow_channel.makeFunctionControllableIfConstant(f_D_fn_name, "f");

  const std::string class_name = "ADWallFrictionFunctionMaterial";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<SubdomainName>>("block") = flow_channel.getSubdomainNames();
  params.set<MaterialPropertyName>("f_D") = FlowModelSinglePhase::FRICTION_FACTOR_DARCY;
  params.set<FunctionName>("function") = f_D_fn_name;
  _sim.addMaterial(class_name, genName(flow_channel.name(), "f_wall_fn_mat"), params);
}

void
Closures1PhaseBase::addAverageWallTemperatureMaterial(const FlowChannel1Phase & flow_channel) const
{
  const std::string class_name = "ADAverageWallTemperature3EqnMaterial";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<SubdomainName>>("block") = flow_channel.getSubdomainNames();
  params.set<std::vector<VariableName>>("T_wall_sources") = flow_channel.getWallTemperatureNames();
  params.set<std::vector<MaterialPropertyName>>("Hw_sources") =
      flow_channel.getWallHTCNames1Phase();
  params.set<std::vector<VariableName>>("P_hf_sources") = flow_channel.getHeatedPerimeterNames();
  params.set<std::vector<VariableName>>("P_hf_total") = {FlowModel::HEAT_FLUX_PERIMETER};
  params.set<std::vector<VariableName>>("T_fluid") = {FlowModelSinglePhase::TEMPERATURE};
  _sim.addMaterial(class_name, genName(flow_channel.name(), "avg_T_wall_3eqn_mat"), params);
}
