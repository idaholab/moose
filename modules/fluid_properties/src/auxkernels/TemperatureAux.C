//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TemperatureAux.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("FluidPropertiesApp", TemperatureAux);

InputParameters
TemperatureAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredCoupledVar("e", "Specific internal energy");
  params.addRequiredCoupledVar("v", "Specific volume");
  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");
  params.addClassDescription(
      "Computes temperature given specific volume and specific internal energy");
  return params;
}

TemperatureAux::TemperatureAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _v(coupledValue("v")),
    _e(coupledValue("e")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

Real
TemperatureAux::computeValue()
{
  return _fp.T_from_v_e(_v[_qp], _e[_qp]);
}
