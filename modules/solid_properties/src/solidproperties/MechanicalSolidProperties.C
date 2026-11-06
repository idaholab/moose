//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MechanicalSolidProperties.h"

registerMooseObject("SolidPropertiesApp", MechanicalSolidProperties);

InputParameters
MechanicalSolidProperties::validParams()
{
  InputParameters params = SolidProperties::validParams();
  params.addClassDescription(
      "Base class for mechanical solid properties as a function of temperature.");
  return params;
}

MechanicalSolidProperties::MechanicalSolidProperties(const InputParameters & parameters)
  : SolidProperties(parameters)
{
}
