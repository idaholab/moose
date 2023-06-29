//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalSolidProperties.h"

InputParameters
ThermalSolidProperties::validParams()
{
  InputParameters params = SolidProperties::validParams();
  return params;
}

ThermalSolidProperties::ThermalSolidProperties(const InputParameters & parameters)
  : SolidProperties(parameters)
{
}
