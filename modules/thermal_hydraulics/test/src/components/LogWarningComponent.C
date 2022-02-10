//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LogWarningComponent.h"

registerMooseObject("ThermalHydraulicsTestApp", LogWarningComponent);

InputParameters
LogWarningComponent::validParams()
{
  InputParameters params = Component::validParams();
  params.addClassDescription("Component that logs a warning.");
  return params;
}

LogWarningComponent::LogWarningComponent(const InputParameters & params) : Component(params)
{
  logWarning("This is a warning.");
}
