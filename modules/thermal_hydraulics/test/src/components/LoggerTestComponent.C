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
  params.addRequiredParam<bool>("log_errors", "True to log some errors.");
  params.addRequiredParam<bool>("log_warnings", "True to log some warnings.");
  params.addClassDescription("Component for testing Logger.");
  return params;
}

LoggerTestComponent::LoggerTestComponent(const InputParameters & params)
  : Component(params),
    _log_errors(getParam<bool>("log_errors")),
    _log_warnings(getParam<bool>("log_warnings"))
{
  if (_log_errors)
    logError("error 1");

  if (_log_warnings)
    logWarning("warning 1");

  if (_log_errors)
    logError("error 2");

  if (_log_warnings)
    logWarning("warning 2");
}
