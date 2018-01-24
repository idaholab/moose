/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowFluidStateWaterNCG.h"
#include "PorousFlowCapillaryPressure.h"
#include "PorousFlowWaterNCG.h"

template <>
InputParameters
validParams<PorousFlowFluidStateWaterNCG>()
{
  InputParameters params = validParams<PorousFlowFluidStateFlashBase>();
  params.addClassDescription("Fluid state class for water and non-condensable gas");
  return params;
}

PorousFlowFluidStateWaterNCG::PorousFlowFluidStateWaterNCG(const InputParameters & parameters)
  : PorousFlowFluidStateFlashBase(parameters),
    _fs_uo(getUserObject<PorousFlowWaterNCG>("fluid_state"))
{
  // Check that a valid Water-NCG FluidState has been supplied in fluid_state
  if (_fs_uo.fluidStateName() != "water-ncg")
    mooseError("Only a valid Water-NCG FluidState can be used in ", _name);
}

void
PorousFlowFluidStateWaterNCG::thermophysicalProperties()
{
  // The FluidProperty objects use temperature in K
  Real Tk = _temperature[_qp] + _T_c2k;

  _fs_uo.thermophysicalProperties(_gas_porepressure[_qp], Tk, (*_z[0])[_qp], _fsp);
}
