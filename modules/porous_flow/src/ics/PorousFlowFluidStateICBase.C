//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowFluidStateICBase.h"
#include "PorousFlowDictator.h"

template <>
InputParameters
validParams<PorousFlowFluidStateICBase>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addRequiredCoupledVar("gas_porepressure",
                               "Variable that is the porepressure of the gas phase");
  params.addRequiredCoupledVar("temperature", "Variable that is the fluid temperature");
  MooseEnum unit_choice("Kelvin=0 Celsius=1", "Kelvin");
  params.addParam<MooseEnum>(
      "temperature_unit", unit_choice, "The unit of the temperature variable");
  params.addCoupledVar("saturation", 0.0, "Gas saturation");
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names");
  params.addClassDescription("An initial condition to calculate z from saturation");
  return params;
}

PorousFlowFluidStateICBase::PorousFlowFluidStateICBase(const InputParameters & parameters)
  : InitialCondition(parameters),
    _gas_porepressure(coupledValue("gas_porepressure")),
    _temperature(coupledValue("temperature")),
    _saturation(coupledValue("saturation")),
    _T_c2k(getParam<MooseEnum>("temperature_unit") == 0 ? 0.0 : 273.15),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator"))
{
}
