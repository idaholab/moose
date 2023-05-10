//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatRateHeatFlux.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", HeatRateHeatFlux);

InputParameters
HeatRateHeatFlux::validParams()
{
  InputParameters params = FunctionSideIntegral::validParams();

  params.addParam<FunctionName>("scale", 1.0, "Function by which to scale the heat flux");

  params.renameParam("function", "q", "Heat flux function");

  params.addClassDescription("Integrates a heat flux function over a boundary");

  return params;
}

HeatRateHeatFlux::HeatRateHeatFlux(const InputParameters & parameters)
  : FunctionSideIntegral(parameters),

    _scale_fn(getFunction("scale"))
{
}

Real
HeatRateHeatFlux::computeQpIntegral()
{
  return _scale_fn.value(_t, _q_point[_qp]) * FunctionSideIntegral::computeQpIntegral();
}
