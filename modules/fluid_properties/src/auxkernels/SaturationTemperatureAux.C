//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SaturationTemperatureAux.h"
#include "TwoPhaseFluidProperties.h"

registerMooseObject("FluidPropertiesApp", SaturationTemperatureAux);

InputParameters
SaturationTemperatureAux::validParams()
{
  InputParameters params = AuxKernel::validParams();

  params.addClassDescription(
      "Computes saturation temperature from pressure and 2-phase fluid properties object");

  params.addRequiredCoupledVar("p", "Pressure at which to evaluate saturation temperature");
  params.addRequiredParam<UserObjectName>("fp_2phase",
                                          "The name of the user object with fluid properties");

  return params;
}

SaturationTemperatureAux::SaturationTemperatureAux(const InputParameters & parameters)
  : AuxKernel(parameters),

    _p(coupledValue("p")),
    _fp_2phase(getUserObject<TwoPhaseFluidProperties>("fp_2phase"))
{
}

Real
SaturationTemperatureAux::computeValue()
{
  return _fp_2phase.T_sat(_p[_qp]);
}
