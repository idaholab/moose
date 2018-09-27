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

registerMooseObject("NavierStokesApp", TemperatureAux);

template <>
InputParameters
validParams<TemperatureAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("specific_volume", "Specific volume");
  params.addRequiredCoupledVar("specific_internal_energy", "Specific internal energy");
  params.addRequiredParam<UserObjectName>("fp", "The name of fluid properties user object to use.");
  return params;
}

TemperatureAux::TemperatureAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _s_volume(coupledValue("specific_volume")),
    _s_internal_energy(coupledValue("specific_internal_energy")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

Real
TemperatureAux::computeValue()
{
  return _fp.T_from_v_e(_s_volume[_qp], _s_internal_energy[_qp]);
}
