/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowFluidStateBrineCO2IC.h"
#include "PorousFlowBrineCO2.h"

template <>
InputParameters
validParams<PorousFlowFluidStateBrineCO2IC>()
{
  InputParameters params = validParams<PorousFlowFluidStateICBase>();
  params.addRequiredParam<UserObjectName>("fluid_state", "Name of the FluidState UserObject");
  params.addCoupledVar("xnacl", 0, "The salt mass fraction in the brine (kg/kg)");
  params.addClassDescription(
      "An initial condition to calculate z from saturation for brine and CO2");
  return params;
}

PorousFlowFluidStateBrineCO2IC::PorousFlowFluidStateBrineCO2IC(const InputParameters & parameters)
  : PorousFlowFluidStateICBase(parameters),
    _xnacl(coupledValue("xnacl")),
    _fs_uo(getUserObject<PorousFlowBrineCO2>("fluid_state"))
{
  // Check that a valid brine-CO2 FluidState has been supplied in fluid_state
  if (_fs_uo.fluidStateName() != "brine-co2")
    mooseError("Only a valid Brine-CO2 FluidState can be used in ", _name);
}

Real
PorousFlowFluidStateBrineCO2IC::value(const Point & /*p*/)
{
  // The brine-co2 fluid state needs temperature in K
  Real Tk = _temperature[_qp] + _T_c2k;

  // The total mass fraction corresponding to the input saturation
  Real z = _fs_uo.totalMassFraction(_gas_porepressure[_qp], Tk, _xnacl[_qp], _saturation[_qp]);

  return z;
}
