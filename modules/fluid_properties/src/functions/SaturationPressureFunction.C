//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SaturationPressureFunction.h"
#include "TwoPhaseFluidProperties.h"

registerMooseObject("FluidPropertiesApp", SaturationPressureFunction);

InputParameters
SaturationPressureFunction::validParams()
{
  InputParameters params = Function::validParams();

  params.addRequiredParam<FunctionName>("T", "Temperature function");
  params.addRequiredParam<UserObjectName>("fp_2phase", "2-phase fluid properties");

  params.addClassDescription(
      "Computes saturation pressure from temperature function and 2-phase fluid properties object");

  return params;
}

SaturationPressureFunction::SaturationPressureFunction(const InputParameters & parameters)
  : Function(parameters),
    FunctionInterface(this),

    _T_fn(getFunction("T"))
{
}

void
SaturationPressureFunction::initialSetup()
{
  _fp_2phase = &getUserObject<TwoPhaseFluidProperties>("fp_2phase");
}

Real
SaturationPressureFunction::value(Real t, const Point & point) const
{
  return _fp_2phase->p_sat(_T_fn.value(t, point));
}

RealVectorValue
SaturationPressureFunction::gradient(Real t, const Point & point) const
{
  return _T_fn.gradient(t, point) / _fp_2phase->dT_sat_dp(value(t, point));
}
