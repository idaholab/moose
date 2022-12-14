//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatSourceVolumetric1Phase.h"
#include "FlowChannel1Phase.h"
#include "FlowModelSinglePhase.h"

registerMooseObject("ThermalHydraulicsApp", HeatSourceVolumetric1Phase);

InputParameters
HeatSourceVolumetric1Phase::validParams()
{
  InputParameters params = Component::validParams();
  params.addRequiredParam<std::string>("flow_channel",
                                       "Flow channel name in which to apply heat source");
  params.addRequiredParam<FunctionName>("q", "Volumetric heat source [W/m^3]");
  params.addClassDescription("Volumetric heat source applied on a flow channel");
  return params;
}

HeatSourceVolumetric1Phase::HeatSourceVolumetric1Phase(const InputParameters & parameters)
  : Component(parameters)
{
}

void
HeatSourceVolumetric1Phase::check() const
{
  Component::check();

  checkComponentOfTypeExists<FlowChannel1Phase>("flow_channel");
}

void
HeatSourceVolumetric1Phase::addMooseObjects()
{
  const FlowChannelBase & fch = getComponent<FlowChannel1Phase>("flow_channel");

  {
    std::string class_name = "OneD3EqnEnergyHeatSource";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
    pars.set<std::vector<SubdomainName>>("block") = fch.getSubdomainNames();
    pars.set<FunctionName>("q") = getParam<FunctionName>("q");
    pars.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    getTHMProblem().addKernel(class_name, genName(name(), "rhoE_heat_source"), pars);
  }
}
