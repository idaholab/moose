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
InjectionWell::addControlLogic()
{
  // m_dot
  const std::string get_mdot_fn_name = name() + "_get_inlet_mdot_ctrl";
  {
    const std::string class_name = "GetFunctionValueControl";
    auto params = _factory.getValidParams(class_name);
    params.set<FunctionName>("function") = getParam<FunctionName>("inlet_mass_flow_rate");
    params.set<Point>("point") = _surface_point;
    addControlLogicObject(class_name, get_mdot_fn_name, params);
  }
  {
    const std::string class_name = "SetComponentRealValueControl";
    auto params = _factory.getValidParams(class_name);
    params.set<std::string>("component") = inletName();
    params.set<std::string>("parameter") = "m_dot";
    params.set<std::string>("value") = get_mdot_fn_name + ":value";
    addControlLogicObject(class_name, name() + "_set_inlet_mdot_ctrl", params);
  }

  // T
  const std::string get_T_fn_name = name() + "_get_inlet_T_ctrl";
  {
    const std::string class_name = "GetFunctionValueControl";
    auto params = _factory.getValidParams(class_name);
    params.set<FunctionName>("function") = getParam<FunctionName>("inlet_temperature");
    params.set<Point>("point") = _surface_point;
    addControlLogicObject(class_name, get_T_fn_name, params);
  }
  {
    const std::string class_name = "SetComponentRealValueControl";
    auto params = _factory.getValidParams(class_name);
    params.set<std::string>("component") = inletName();
    params.set<std::string>("parameter") = "T";
    params.set<std::string>("value") = get_T_fn_name + ":value";
    addControlLogicObject(class_name, name() + "_set_inlet_T_ctrl", params);
  }
}

void
InjectionWell::addInlet()
{
  const std::string class_name = "InletMassFlowRateTemperature1Phase";
  auto params = _factory.getValidParams(class_name);
  params.set<BoundaryName>("input") = flowChannelName(0) + ":in";
  params.set<Real>("m_dot") = 0.0; // arbitrary placeholder value; this gets controlled
  params.set<Real>("T") = 300.0;   // arbitrary placeholder value; this gets controlled
  addTHMComponent(class_name, inletName(), params);
}

std::string
InjectionWell::inletName() const
{
  return name() + "_inlet";
}
