//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SoundSpeedAux.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("ThermalHydraulicsApp", SoundSpeedAux);

InputParameters
SoundSpeedAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredCoupledVar("v", "specific volume");
  params.addRequiredCoupledVar("e", "specific internal energy");
  params.addRequiredParam<UserObjectName>("fp", "The name of fluid properties object to use.");

  return params;
}

SoundSpeedAux::SoundSpeedAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _v(coupledValue("v")),
    _e(coupledValue("e")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

Real
SoundSpeedAux::computeValue()
{
  return _fp.c_from_v_e(_v[_qp], _e[_qp]);
}
