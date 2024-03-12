//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LoggerTestComponent.h"

registerMooseObject("ThermalHydraulicsTestApp", LoggerTestComponent);

InputParameters
LoggerTestComponent::validParams()
{
  InputParameters params = Component::validParams();
  params.addClassDescription("Component for testing Logger.");
  return params;
}

LoggerTestComponent::LoggerTestComponent(const InputParameters & params) : Component(params)
{
  logWarning("This is a warning.");
}
