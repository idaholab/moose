//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowFluidStateBrineCO2.h"
#include "PorousFlowCapillaryPressure.h"
#include "PorousFlowBrineCO2.h"

template <>
InputParameters
validParams<PorousFlowFluidStateBrineCO2>()
{
  InputParameters params = validParams<PorousFlowFluidStateFlashBase>();
  params.addCoupledVar("xnacl", 0, "The salt mass fraction in the brine (kg/kg)");
  params.addClassDescription("Fluid state class for brine and CO2");
  return params;
}

PorousFlowFluidStateBrineCO2::PorousFlowFluidStateBrineCO2(const InputParameters & parameters)
  : PorousFlowFluidStateFlashBase(parameters),
    _xnacl(_nodal_material ? coupledNodalValue("xnacl") : coupledValue("xnacl")),
    _fs_uo(getUserObject<PorousFlowBrineCO2>("fluid_state"))
{
  // Check that a valid Brine-CO2 FluidState has been supplied in fluid_state
  if (_fs_uo.fluidStateName() != "brine-co2")
    mooseError("Only a valid Brine-CO2 FluidState can be used in ", _name);
}

void
PorousFlowFluidStateBrineCO2::thermophysicalProperties()
{
  // The FluidProperty objects use temperature in K
  Real Tk = _temperature[_qp] + _T_c2k;

  _fs_uo.thermophysicalProperties(_gas_porepressure[_qp], Tk, _xnacl[_qp], (*_z[0])[_qp], _fsp);
}
