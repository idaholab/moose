//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComponentGroup.h"

registerMooseObject("ThermalHydraulicsApp", ComponentGroup);

InputParameters
ComponentGroup::validParams()
{
  InputParameters params = THMObject::validParams();
  params.addClassDescription("Group of components. Used only for parsing input files.");
  params.addPrivateParam<std::string>("built_by_action", "add_component");
  params.registerBase("Component");
  return params;
}

ComponentGroup::ComponentGroup(const InputParameters & parameters) : THMObject(parameters) {}
