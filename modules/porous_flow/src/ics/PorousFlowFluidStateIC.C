//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowFluidStateIC.h"
#include "PorousFlowDictator.h"
#include "PorousFlowFluidStateMultiComponentBase.h"

registerMooseObject("PorousFlowApp", PorousFlowFluidStateIC);

InputParameters
PorousFlowFluidStateIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addRequiredCoupledVar("gas_porepressure",
                               "Variable that is the porepressure of the gas phase");
  params.addRequiredCoupledVar("temperature", "Variable that is the fluid temperature");
  MooseEnum unit_choice("Kelvin=0 Celsius=1", "Kelvin");
  params.addParam<MooseEnum>(
      "temperature_unit", unit_choice, "The unit of the temperature variable");
  params.addCoupledVar("saturation", 0.0, "Gas saturation");
  params.addRequiredParam<UserObjectName>("fluid_state", "Name of the FluidState UserObject");
  params.addCoupledVar("xnacl", 0, "The salt mass fraction in the brine (kg/kg)");
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names");
  params.addClassDescription("An initial condition to calculate z from saturation");
  return params;
}

PorousFlowFluidStateIC::PorousFlowFluidStateIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _gas_porepressure(coupledValue("gas_porepressure")),
    _temperature(coupledValue("temperature")),
    _Xnacl(coupledValue("xnacl")),
    _saturation(coupledValue("saturation")),
    _T_c2k(getParam<MooseEnum>("temperature_unit") == 0 ? 0.0 : 273.15),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _fs(getUserObject<PorousFlowFluidStateMultiComponentBase>("fluid_state"))
{
}

Real
PorousFlowFluidStateIC::value(const Point & /*p*/)
{
  // The fluid state user object needs temperature in K
  const Real Tk = _temperature[_qp] + _T_c2k;

  // The total mass fraction corresponding to the input saturation
  return _fs.totalMassFraction(_gas_porepressure[_qp], Tk, _Xnacl[_qp], _saturation[_qp], _qp);
}
