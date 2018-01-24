//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StagnationTemperatureAux.h"
#include "SinglePhaseFluidProperties.h"

template <>
InputParameters
validParams<StagnationTemperatureAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("e", "Specific internal energy");
  params.addRequiredCoupledVar("v", "Specific volume");
  params.addRequiredCoupledVar("vel", "Velocity");
  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");
  params.addClassDescription("Computes stagnation temperature from specific volume, specific "
                             "internal energy, and velocity");
  return params;
}

StagnationTemperatureAux::StagnationTemperatureAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _specific_volume(coupledValue("v")),
    _specific_internal_energy(coupledValue("e")),
    _velocity(coupledValue("vel")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

Real
StagnationTemperatureAux::computeValue()
{
  // static properties
  const Real v = _specific_volume[_qp];
  const Real e = _specific_internal_energy[_qp];
  const Real u = _velocity[_qp];
  const Real p = _fp.pressure(v, e);

  // static entropy is equal to stagnation entropy by definition of the stagnation state
  const Real s = _fp.s(v, e);

  // stagnation properties
  const Real h0 = e + p * v + 0.5 * u * u;
  const Real p0 = _fp.p_from_h_s(h0, s);
  Real rho0, e0;
  _fp.rho_e_ps(p0, s, rho0, e0);

  return _fp.temperature(1.0 / rho0, e0);
}
