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
ProductionWell::addOutlet()
{
  const std::string class_name = "Outlet1Phase";
  auto params = _factory.getValidParams(class_name);
  params.set<BoundaryName>("input") = flowChannelName(0) + ":out";
  params.set<FunctionName>("p") = getParam<FunctionName>("outlet_pressure");
  addTHMComponent(class_name, outletName(), params);
}

std::string
ProductionWell::outletName() const
{
  return name() + "_outlet";
}
