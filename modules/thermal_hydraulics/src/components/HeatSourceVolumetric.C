//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatSourceVolumetric.h"

registerMooseObject("ThermalHydraulicsApp", HeatSourceVolumetric);

InputParameters
HeatSourceVolumetric::validParams()
{
  InputParameters params = Component::validParams();
  params.addRequiredParam<std::string>("flow_channel",
                                       "Flow channel name in which to apply heat source");
  params.addRequiredParam<FunctionName>("q", "Volumetric heat source [W/m^3]");
  params.addClassDescription("Volumetric heat source applied on a flow channel");
  return params;
}

HeatSourceVolumetric::HeatSourceVolumetric(const InputParameters & parameters)
  : Component(parameters)
{
  logError("Deprecated component. Use HeatSourceVolumetric1Phase or HeatSourceVolumetric2Phase "
           "instead.");
}
