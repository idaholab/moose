//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatTransferFromExternalAppTemperature1Phase.h"

registerMooseObject("ThermalHydraulicsApp", HeatTransferFromExternalAppTemperature1Phase);

InputParameters
HeatTransferFromExternalAppTemperature1Phase::validParams()
{
  InputParameters params = HeatTransferFromTemperature1Phase::validParams();
  params.addParam<FunctionName>("initial_T_wall", "Initial condition for wall temperature [K]");
  params.addParam<VariableName>("T_ext", "Name of the wall temperature variable");
  params.addClassDescription("Heat transfer into 1-phase flow channel from temperature provided by "
                             "an external application");
  return params;
}

HeatTransferFromExternalAppTemperature1Phase::HeatTransferFromExternalAppTemperature1Phase(
    const InputParameters & parameters)
  : HeatTransferFromTemperature1Phase(parameters)
{
}

void
HeatTransferFromExternalAppTemperature1Phase::initSecondary()
{
  HeatTransferFromTemperature1Phase::initSecondary();

  if (isParamValid("T_ext"))
    _T_wall_name = getParam<VariableName>("T_ext");
}

void
HeatTransferFromExternalAppTemperature1Phase::check() const
{
  HeatTransferFromTemperature1Phase::check();

  if (!isParamValid("T_ext"))
    if (!isParamValid("initial_T_wall") && !_app.isRestarting())
      logError("Missing initial condition for wall temperature.");
}

void
HeatTransferFromExternalAppTemperature1Phase::addVariables()
{
  HeatTransferFromTemperature1Phase::addVariables();

  if (!isParamValid("T_ext"))
    if (isParamValid("initial_T_wall"))
      getTHMProblem().addFunctionIC(
          _T_wall_name, getParam<FunctionName>("initial_T_wall"), _flow_channel_subdomains);
}

void
HeatTransferFromExternalAppTemperature1Phase::addMooseObjects()
{
  HeatTransferFromTemperature1Phase::addMooseObjects();

  addHeatTransferKernels();
}
