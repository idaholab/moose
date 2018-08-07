//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OptionallyVectorCoupledForce.h"

registerMooseObject("MooseTestApp", OptionallyVectorCoupledForce);

template <>
InputParameters
validParams<OptionallyVectorCoupledForce>()
{
  InputParameters params = validParams<Kernel>();

  params.addCoupledVar("v", {1, 2}, "The two coupled variables which provide the force");

  return params;
}

OptionallyVectorCoupledForce::OptionallyVectorCoupledForce(const InputParameters & parameters)
  : Kernel(parameters)
{
  _v_var.resize(coupledComponents("v"));
  _v.resize(coupledComponents("v"));
  for (unsigned int j = 0; j < coupledComponents("v"); ++j)
  {
    _v_var[j] = coupled("v", j);
    _v[j] = &coupledValue("v", j);
  }
}

Real
OptionallyVectorCoupledForce::computeQpResidual()
{
  Real s = 0;
  for (unsigned int j = 0; j < _v.size(); ++j)
    s += (*_v[j])[_qp];
  return -s * _test[_i][_qp];
}

Real
OptionallyVectorCoupledForce::computeQpJacobian()
{
  return 0;
}
