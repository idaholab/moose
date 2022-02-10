//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PrandtlNumberAux.h"
#include "SinglePhaseFluidProperties.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", PrandtlNumberAux);

InputParameters
PrandtlNumberAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredCoupledVar("v", "Specific volume");
  params.addRequiredCoupledVar("e", "Specific internal energy");
  params.addRequiredParam<UserObjectName>("fp",
                                          "The name of the user object with fluid properties");
  return params;
}

PrandtlNumberAux::PrandtlNumberAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _v(coupledValue("v")),
    _e(coupledValue("e")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

Real
PrandtlNumberAux::computeValue()
{
  Real cp = _fp.cp_from_v_e(_v[_qp], _e[_qp]);
  Real mu = _fp.mu_from_v_e(_v[_qp], _e[_qp]);
  Real k = _fp.k_from_v_e(_v[_qp], _e[_qp]);
  return THM::Prandtl(cp, mu, k);
}
