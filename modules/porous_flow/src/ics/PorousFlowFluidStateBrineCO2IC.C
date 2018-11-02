//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowFluidStateBrineCO2IC.h"
#include "PorousFlowBrineCO2.h"

registerMooseObject("PorousFlowApp", PorousFlowFluidStateBrineCO2IC);

template <>
InputParameters
validParams<PorousFlowFluidStateBrineCO2IC>()
{
  InputParameters params = validParams<PorousFlowFluidStateIC>();
  params.addClassDescription(
      "An initial condition to calculate z from saturation for brine and CO2");
  return params;
}

PorousFlowFluidStateBrineCO2IC::PorousFlowFluidStateBrineCO2IC(const InputParameters & parameters)
  : PorousFlowFluidStateIC(parameters)
{
  mooseDeprecated(name(),
                  ": the PorousFlowFluidStateBrineCO2IC IC is deprecated. Just use "
                  "PorousFlowFluidStateIC instead");
}
