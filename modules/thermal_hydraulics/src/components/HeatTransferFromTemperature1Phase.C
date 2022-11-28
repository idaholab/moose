//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatTransferFromTemperature1Phase.h"
#include "FlowModelSinglePhase.h"

InputParameters
HeatTransferFromTemperature1Phase::validParams()
{
  InputParameters params = HeatTransfer1PhaseBase::validParams();
  MooseEnum var_type("nodal elemental", "nodal", false);
  params.addParam<MooseEnum>(
      "var_type", var_type, "The type of wall temperature variable (nodal, elemental).");
  return params;
}

HeatTransferFromTemperature1Phase::HeatTransferFromTemperature1Phase(
    const InputParameters & parameters)
  : HeatTransfer1PhaseBase(parameters),
    _fe_type(getParam<MooseEnum>("var_type") == 0 ? FEType(FIRST, LAGRANGE)
                                                  : FEType(CONSTANT, MONOMIAL))
{
}

const FEType &
HeatTransferFromTemperature1Phase::getFEType()
{
  return _fe_type;
}

void
HeatTransferFromTemperature1Phase::addVariables()
{
  HeatTransfer1PhaseBase::addVariables();

  getTHMProblem().addSimVariable(false, _T_wall_name, getFEType(), _flow_channel_subdomains);
}

void
HeatTransferFromTemperature1Phase::addMooseObjects()
{
  HeatTransfer1PhaseBase::addMooseObjects();
}

void
HeatTransferFromTemperature1Phase::addHeatTransferKernels()
{
  {
    const std::string class_name = "ADOneDEnergyWallHeating";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel_subdomains;
    params.set<std::vector<VariableName>>("T_wall") = {_T_wall_name};
    params.set<MaterialPropertyName>("Hw") = _Hw_1phase_name;
    params.set<std::vector<VariableName>>("P_hf") = {_P_hf_name};
    params.set<MaterialPropertyName>("T") = FlowModelSinglePhase::TEMPERATURE;
    getTHMProblem().addKernel(class_name, genName(name(), "wall_heat_transfer"), params);
  }
}

bool
HeatTransferFromTemperature1Phase::isTemperatureType() const
{
  return true;
}
