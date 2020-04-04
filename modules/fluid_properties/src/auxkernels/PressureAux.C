//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PressureAux.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("FluidPropertiesApp", PressureAux);

InputParameters
PressureAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredCoupledVar("e", "Specific internal energy");
  params.addRequiredCoupledVar("v", "Specific volume");
  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");
  params.addClassDescription(
      "Computes pressure given specific volume and specific internal energy");
  return params;
}

PressureAux::PressureAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _v(coupledValue("v")),
    _e(coupledValue("e")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

Real
PressureAux::computeValue()
{
  return _fp.p_from_v_e(_v[_qp], _e[_qp]);
}
