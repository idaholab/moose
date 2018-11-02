//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowFluidStateWaterNCGIC.h"
#include "PorousFlowWaterNCG.h"

registerMooseObject("PorousFlowApp", PorousFlowFluidStateWaterNCGIC);

template <>
InputParameters
validParams<PorousFlowFluidStateWaterNCGIC>()
{
  InputParameters params = validParams<PorousFlowFluidStateIC>();
  params.addClassDescription(
      "An initial condition to calculate z from saturation for water and non-condensable gas");
  return params;
}

PorousFlowFluidStateWaterNCGIC::PorousFlowFluidStateWaterNCGIC(const InputParameters & parameters)
  : PorousFlowFluidStateIC(parameters)
{
  mooseDeprecated(name(),
                  ": the PorousFlowFluidStateWaterNCGIC IC is deprecated. Just use "
                  "PorousFlowFluidStateIC instead");
}
