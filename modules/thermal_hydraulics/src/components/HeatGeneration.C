//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatGeneration.h"

registerMooseObject("ThermalHydraulicsApp", HeatGeneration);

InputParameters
HeatGeneration::validParams()
{
  InputParameters params = Component::validParams();
  params.addRequiredParam<std::string>("hs", "Heat structure in which to apply heat source");
  params.addRequiredParam<std::vector<std::string>>(
      "regions", "Heat structure regions where heat generation is to be applied");
  params.addParam<std::string>("power", "Reactor power component");
  params.addParam<Real>(
      "power_fraction", 1., "Fraction of reactor power that goes into the heat structure [-]");
  params.addParam<FunctionName>("power_shape_function", "Axial power shape [-]");
  params.addParam<VariableName>("power_density", "Power density variable");

  return params;
}

HeatGeneration::HeatGeneration(const InputParameters & parameters) : Component(parameters)
{
  logError("'HeatGeneration' component is deprecated, use 'HeatSourceFromTotalPower' or "
           "'HeatSourceFromPowerDensity' instead.");
}
