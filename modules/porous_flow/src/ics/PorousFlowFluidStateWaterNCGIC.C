/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowFluidStateWaterNCGIC.h"
#include "PorousFlowWaterNCG.h"

template <>
InputParameters
validParams<PorousFlowFluidStateWaterNCGIC>()
{
  InputParameters params = validParams<PorousFlowFluidStateICBase>();
  params.addRequiredParam<UserObjectName>("fluid_state", "Name of the FluidState UserObject");
  params.addClassDescription(
      "An initial condition to calculate z from saturation for water and non-condensable gas");
  return params;
}

PorousFlowFluidStateWaterNCGIC::PorousFlowFluidStateWaterNCGIC(const InputParameters & parameters)
  : PorousFlowFluidStateICBase(parameters), _fs_uo(getUserObject<PorousFlowWaterNCG>("fluid_state"))
{
  // Check that a valid Water-NCG FluidState has been supplied in fluid_state
  if (_fs_uo.fluidStateName() != "water-ncg")
    mooseError("Only a valid Water-NCG FluidState can be used in ", _name);
}

Real
PorousFlowFluidStateWaterNCGIC::value(const Point & /*p*/)
{
  // The water-ncg fluid state needs temperature in K
  Real Tk = _temperature[_qp] + _T_c2k;

  // The total mass fraction corresponding to the input saturation
  Real z = _fs_uo.totalMassFraction(_gas_porepressure[_qp], Tk, _saturation[_qp]);

  return z;
}
