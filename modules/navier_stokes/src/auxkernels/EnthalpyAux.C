//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EnthalpyAux.h"

registerMooseObject("NavierStokesApp", EnthalpyAux);

template <>
InputParameters
validParams<EnthalpyAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("rho", "Density");
  params.addRequiredCoupledVar("rho_et", "Total energy");
  params.addCoupledVar("pressure", "Coupled value pressure");

  return params;
}

EnthalpyAux::EnthalpyAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _rho(coupledValue("rho")),
    _rho_et(coupledValue("rho_et")),
    _pressure(coupledValue("pressure"))
{
}

Real
EnthalpyAux::computeValue()
{
  return (_rho_et[_qp] + _pressure[_qp]) / _rho[_qp];
}
