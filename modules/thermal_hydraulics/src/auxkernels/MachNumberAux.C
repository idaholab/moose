//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MachNumberAux.h"
#include "SinglePhaseFluidProperties.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", MachNumberAux);

InputParameters
MachNumberAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Computes Mach number.");
  params.addRequiredCoupledVar("vel", "x-component of phase velocity");
  params.addRequiredCoupledVar("v", "Specific volume");
  params.addRequiredCoupledVar("e", "Specific internal energy");
  params.addRequiredParam<UserObjectName>("fp", "The name of fluid properties object to use.");
  return params;
}

MachNumberAux::MachNumberAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _vel(coupledValue("vel")),
    _v(coupledValue("v")),
    _e(coupledValue("e")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

Real
MachNumberAux::computeValue()
{
  Real speed_of_sound = _fp.c_from_v_e(_v[_qp], _e[_qp]);
  return _vel[_qp] / speed_of_sound;
}
