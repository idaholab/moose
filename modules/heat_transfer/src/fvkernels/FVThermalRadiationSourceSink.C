//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVThermalRadiationSourceSink.h"
#include "MathUtils.h"
#include "HeatConductionNames.h"

registerMooseObject("HeatTransferApp", FVThermalRadiationSourceSink);

InputParameters
FVThermalRadiationSourceSink::validParams()
{
  InputParameters params = FVElementalKernel::validParams();

  params.addClassDescription(
      "Implements the source and the sink terms for radiation heat transfer.");
  params.addRequiredParam<MooseFunctorName>("temperature_radiation", "The radiation temperature.");
  params.addParam<MooseFunctorName>("opacity", 1.0, "The opacity field.");

  return params;
}

FVThermalRadiationSourceSink::FVThermalRadiationSourceSink(const InputParameters & parameters)
  : FVElementalKernel(parameters),
    _temperature_radiation(getFunctor<ADReal>("temperature_radiation")),
    _opacity(getFunctor<ADReal>("opacity"))
{
}

ADReal
FVThermalRadiationSourceSink::computeQpResidual()
{
  return _opacity(makeElemArg(_current_elem), determineState()) *
         (_var(makeElemArg(_current_elem), determineState()) -
          HeatConduction::Constants::sigma * Utility::pow<4>(_temperature_radiation(
                                                 makeElemArg(_current_elem), determineState())));
}
