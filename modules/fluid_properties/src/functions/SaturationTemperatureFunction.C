//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SaturationTemperatureFunction.h"
#include "TwoPhaseFluidProperties.h"

registerMooseObject("FluidPropertiesApp", SaturationTemperatureFunction);

InputParameters
SaturationTemperatureFunction::validParams()
{
  InputParameters params = Function::validParams();

  params.addRequiredParam<FunctionName>("p", "Pressure function");
  params.addRequiredParam<UserObjectName>("fp_2phase", "2-phase fluid properties");

  params.addClassDescription(
      "Computes saturation temperature from pressure function and 2-phase fluid properties object");

  return params;
}

SaturationTemperatureFunction::SaturationTemperatureFunction(const InputParameters & parameters)
  : Function(parameters),
    FunctionInterface(this),

    _p_fn(getFunction("p"))
{
}

void
SaturationTemperatureFunction::initialSetup()
{
  _fp_2phase = &getUserObject<TwoPhaseFluidProperties>("fp_2phase");
}

Real
SaturationTemperatureFunction::value(Real t, const Point & point) const
{
  return _fp_2phase->T_sat(_p_fn.value(t, point));
}

RealVectorValue
SaturationTemperatureFunction::gradient(Real t, const Point & point) const
{
  return _fp_2phase->dT_sat_dp(_p_fn.value(t, point)) * _p_fn.gradient(t, point);
}
