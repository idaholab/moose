//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StagnationPressureAux.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("FluidPropertiesApp", StagnationPressureAux);

InputParameters
StagnationPressureAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredCoupledVar("e", "Specific internal energy");
  params.addRequiredCoupledVar("v", "Specific volume");
  params.addRequiredCoupledVar("vel", "Velocity");
  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");
  params.addClassDescription(
      "Computes stagnation pressure from specific volume, specific internal energy, and velocity");
  return params;
}

StagnationPressureAux::StagnationPressureAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _specific_volume(coupledValue("v")),
    _specific_internal_energy(coupledValue("e")),
    _velocity(coupledValue("vel")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

Real
StagnationPressureAux::computeValue()
{
  // static properties
  const Real v = _specific_volume[_qp];
  const Real e = _specific_internal_energy[_qp];
  const Real u = _velocity[_qp];
  const Real p = _fp.p_from_v_e(v, e);

  // static entropy is equal to stagnation entropy by definition of the stagnation state
  const Real s = _fp.s_from_v_e(v, e);

  // stagnation enthalpy
  const Real h0 = e + p * v + 0.5 * u * u;

  return _fp.p_from_h_s(h0, s);
}
