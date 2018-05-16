//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowFluidStateWaterNCG.h"
#include "PorousFlowCapillaryPressure.h"
#include "PorousFlowWaterNCG.h"

registerMooseObject("PorousFlowApp", PorousFlowFluidStateWaterNCG);

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
    paramError("fluid_state", "Only a valid Water-NCG FluidState can be used");
}

void
PorousFlowFluidStateWaterNCG::thermophysicalProperties()
{
  // The FluidProperty objects use temperature in K
  Real Tk = _temperature[_qp] + _T_c2k;

  _fs_uo.thermophysicalProperties(_gas_porepressure[_qp], Tk, (*_Z[0])[_qp], _fsp);
}
