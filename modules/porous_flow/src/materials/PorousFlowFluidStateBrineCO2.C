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

registerMooseObject("PorousFlowApp", PorousFlowFluidStateBrineCO2);

template <>
InputParameters
validParams<PorousFlowFluidStateBrineCO2>()
{
  InputParameters params = validParams<PorousFlowFluidState>();
  params.addCoupledVar("xnacl", 0, "The salt mass fraction in the brine (kg/kg)");
  params.addClassDescription("Fluid state class for brine and CO2");
  return params;
}

PorousFlowFluidStateBrineCO2::PorousFlowFluidStateBrineCO2(const InputParameters & parameters)
  : PorousFlowFluidState(parameters)
{
  mooseDeprecated(name(),
                  ": the PorousFlowFluidStateBrineCO2 material is deprecated. Just use "
                  "PorousFlowFluidState instead");
}
