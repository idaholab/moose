//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatTransferFromExternalAppHeatFlux1Phase.h"
#include "FlowModelSinglePhase.h"

registerMooseObject("ThermalHydraulicsApp", HeatTransferFromExternalAppHeatFlux1Phase);

InputParameters
HeatTransferFromExternalAppHeatFlux1Phase::validParams()
{
  InputParameters params = HeatTransfer1PhaseBase::validParams();
  params.addClassDescription("Heat transfer specified by heat flux provided by an external "
                             "application going into 1-phase flow channel.");
  return params;
}

HeatTransferFromExternalAppHeatFlux1Phase::HeatTransferFromExternalAppHeatFlux1Phase(
    const InputParameters & parameters)
  : HeatTransfer1PhaseBase(parameters)
{
}

void
HeatTransferFromExternalAppHeatFlux1Phase::addVariables()
{
  HeatTransfer1PhaseBase::addVariables();

  getTHMProblem().addSimVariable(
      false, _q_wall_name, FEType(CONSTANT, MONOMIAL), _flow_channel_subdomains);
  getTHMProblem().addConstantIC(_q_wall_name, 0, _flow_channel_subdomains);
}

void
HeatTransferFromExternalAppHeatFlux1Phase::addMooseObjects()
{
  HeatTransfer1PhaseBase::addMooseObjects();

  {
    const std::string class_name = "ADCoupledVariableValueMaterial";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = _flow_channel_subdomains;
    params.set<MaterialPropertyName>("prop_name") = _q_wall_name;
    params.set<std::vector<VariableName>>("coupled_variable") = {_q_wall_name};
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
HeatTransferFromExternalAppHeatFlux1Phase::isTemperatureType() const
{
  return false;
}
