//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SaturationDensityFunction.h"
#include "TwoPhaseFluidProperties.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("FluidPropertiesApp", SaturationDensityFunction);

InputParameters
SaturationDensityFunction::validParams()
{
  InputParameters params = Function::validParams();

  params.addRequiredParam<FunctionName>("T", "Temperature function");
  params.addRequiredParam<UserObjectName>("fp_2phase", "2-phase fluid properties");
  params.addRequiredParam<bool>("use_liquid", "Set true to use liquid phase; else use vapor phase");

  params.addClassDescription("Computes saturation density from temperature function");

  return params;
}

SaturationDensityFunction::SaturationDensityFunction(const InputParameters & parameters)
  : Function(parameters),
    FunctionInterface(this),

    _T_fn(getFunction("T")),
    _fp_2phase(getUserObject<TwoPhaseFluidProperties>("fp_2phase")),
    _fp_liquid(getUserObjectByName<SinglePhaseFluidProperties>(_fp_2phase.getLiquidName())),
    _fp_vapor(getUserObjectByName<SinglePhaseFluidProperties>(_fp_2phase.getVaporName())),
    _use_liquid(getParam<bool>("use_liquid"))
{
}

Real
SaturationDensityFunction::value(Real t, const Point & point) const
{
  const Real T = _T_fn.value(t, point);
  const Real p = _fp_2phase.p_sat(T);

  if (_use_liquid)
    return _fp_liquid.rho_from_p_T(p, T);
  else
    return _fp_vapor.rho_from_p_T(p, T);
}
