//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalSolidPropertiesPostprocessor.h"
#include "ThermalSolidProperties.h"

registerMooseObject("SolidPropertiesApp", ThermalSolidPropertiesPostprocessor);

InputParameters
ThermalSolidPropertiesPostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();

  params.addRequiredParam<UserObjectName>("solid_properties", "Solid properties object to query");
  params.addRequiredParam<PostprocessorName>(
      "T", "Temperature post-processor at which to evaluate property");
  MooseEnum property("density=0 specific_heat=1 thermal_conductivity=2");
  params.addRequiredParam<MooseEnum>("property", property, "Which property to compute.");

  params.addClassDescription("Computes a property from a ThermalSolidProperties object.");

  return params;
}

ThermalSolidPropertiesPostprocessor::ThermalSolidPropertiesPostprocessor(
    const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _solid_properties(getUserObject<ThermalSolidProperties>("solid_properties")),
    _T(getPostprocessorValue("T")),
    _property(getParam<MooseEnum>("property").getEnum<Property>())
{
}

void
ThermalSolidPropertiesPostprocessor::initialize()
{
}

void
ThermalSolidPropertiesPostprocessor::execute()
{
}

PostprocessorValue
ThermalSolidPropertiesPostprocessor::getValue()
{
  switch (_property)
  {
    case Property::DENSITY:
      return _solid_properties.rho_from_T(_T);
      break;
    case Property::SPECIFIC_HEAT:
      return _solid_properties.cp_from_T(_T);
      break;
    case Property::THERMAL_CONDUCTIVITY:
      return _solid_properties.k_from_T(_T);
      break;
    default:
      mooseError("Invalid property option.");
  }
}
