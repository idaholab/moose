//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ProductionWell.h"

registerTHMActionComponentTasks("ThermalHydraulicsApp", ProductionWell);
registerActionComponent("ThermalHydraulicsApp", ProductionWell);

InputParameters
ProductionWell::validParams()
{
  InputParameters params = WellBase::validParams();

  params.addRequiredParam<FunctionName>("outlet_pressure", "Outlet pressure function [Pa]");

  params.addClassDescription("Adds the components and controls for a production well.");

  return params;
}

ProductionWell::ProductionWell(const InputParameters & params) : WellBase(params) {}

void
ProductionWell::addTHMComponents()
{
  addWellBaseComponents(true);
  addOutlet();
}

void
ProductionWell::addControlLogic()
{
  const std::string get_fn_name = name() + "_get_outlet_p_ctrl";
  {
    const std::string class_name = "GetFunctionValueControl";
    auto params = _factory.getValidParams(class_name);
    params.set<FunctionName>("function") = getParam<FunctionName>("outlet_pressure");
    params.set<Point>("point") = _surface_point;
    addControlLogicObject(class_name, get_fn_name, params);
  }
  {
    const std::string class_name = "SetComponentRealValueControl";
    auto params = _factory.getValidParams(class_name);
    params.set<std::string>("component") = outletName();
    params.set<std::string>("parameter") = "p";
    params.set<std::string>("value") = get_fn_name + ":value";
    addControlLogicObject(class_name, name() + "_set_outlet_p_ctrl", params);
  }
}

void
ProductionWell::addOutlet()
{
  const std::string class_name = "Outlet1Phase";
  auto params = _factory.getValidParams(class_name);
  params.set<BoundaryName>("input") = flowChannelName(0) + ":out";
  params.set<Real>("p") = 1e5; // arbitrary placeholder value; this gets controlled
  addTHMComponent(class_name, outletName(), params);
}

std::string
ProductionWell::outletName() const
{
  return name() + "_outlet";
}
