//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InjectionWell.h"

registerTHMActionComponentTasks("ThermalHydraulicsApp", InjectionWell);
registerActionComponent("ThermalHydraulicsApp", InjectionWell);

InputParameters
InjectionWell::validParams()
{
  InputParameters params = WellBase::validParams();

  params.addRequiredParam<FunctionName>("inlet_mass_flow_rate",
                                        "Inlet mass flow rate function [kg/s]");
  params.addRequiredParam<FunctionName>("inlet_temperature", "Inlet temperature function [K]");

  params.addClassDescription("Adds the components and controls for an injection well.");

  return params;
}

InjectionWell::InjectionWell(const InputParameters & params) : WellBase(params) {}

void
InjectionWell::addTHMComponents()
{
  addWellBaseComponents(false);
  addInlet();
}

void
InjectionWell::addInlet()
{
  const std::string class_name = "InletMassFlowRateTemperature1Phase";
  auto params = _factory.getValidParams(class_name);
  params.set<BoundaryName>("input") = flowChannelName(0) + ":in";
  params.set<FunctionName>("m_dot") = getParam<FunctionName>("inlet_mass_flow_rate");
  params.set<FunctionName>("T") = getParam<FunctionName>("inlet_temperature");
  addTHMComponent(class_name, inletName(), params);
}

std::string
InjectionWell::inletName() const
{
  return name() + "_inlet";
}
