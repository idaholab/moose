//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatTransferFromHeatFlux1Phase.h"
#include "FlowModelSinglePhase.h"

registerMooseObject("ThermalHydraulicsApp", HeatTransferFromHeatFlux1Phase);

InputParameters
HeatTransferFromHeatFlux1Phase::validParams()
{
  InputParameters params = HeatTransfer1PhaseBase::validParams();
  params.addRequiredParam<FunctionName>("q_wall", "Specified wall heat flux [W/m^2]");
  params.addClassDescription(
      "Heat transfer specified by heat flux going into 1-phase flow channel.");
  return params;
}

HeatTransferFromHeatFlux1Phase::HeatTransferFromHeatFlux1Phase(const InputParameters & parameters)
  : HeatTransfer1PhaseBase(parameters), _q_wall_fn_name(getParam<FunctionName>("q_wall"))
{
}

void
HeatTransferFromHeatFlux1Phase::addMooseObjects()
{
  HeatTransfer1PhaseBase::addMooseObjects();

  {
    const std::string class_name = "ADGenericFunctionMaterial";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = _flow_channel_subdomains;
    params.set<std::vector<std::string>>("prop_names") = {_q_wall_name};
    params.set<std::vector<FunctionName>>("prop_values") = {_q_wall_fn_name};
    getTHMProblem().addMaterial(class_name, genName(name(), "q_wall_material"), params);
  }

  // wall heat transfer kernel
  {
    const std::string class_name = "ADOneDEnergyWallHeatFlux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel_subdomains;
    params.set<MaterialPropertyName>("q_wall") = _q_wall_name;
    params.set<std::vector<VariableName>>("P_hf") = {_P_hf_name};
    getTHMProblem().addKernel(class_name, genName(name(), "wall_heat"), params);
  }
}

bool
HeatTransferFromHeatFlux1Phase::isTemperatureType() const
{
  return false;
}
