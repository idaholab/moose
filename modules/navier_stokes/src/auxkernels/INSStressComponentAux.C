//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSStressComponentAux.h"
#include "MooseMesh.h"

registerMooseObject("NavierStokesApp", INSStressComponentAux);

template <>
InputParameters
validParams<INSStressComponentAux>()
{
  InputParameters params = validParams<AuxKernel>();

  params.addCoupledVar("velocity", "The velocity component");
  params.addCoupledVar("pressure", 0, "The pressure");
  params.addParam<unsigned>("comp", 0, "The component");
  params.addParam<MaterialPropertyName>("mu_name", "mu", "The viscosity");

  return params;
}

INSStressComponentAux::INSStressComponentAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _grad_velocity(isCoupled("velocity") ? coupledGradient("velocity") : _grad_zero),
    _pressure(coupledValue("pressure")),
    _comp(getParam<unsigned>("comp")),
    _mu(getMaterialProperty<Real>("mu_name"))
{
}

Real
INSStressComponentAux::computeValue()
{
  return _pressure[_qp] - _mu[_qp] * _grad_velocity[_qp](_comp);
}
