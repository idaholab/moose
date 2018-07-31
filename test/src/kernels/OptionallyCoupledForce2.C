//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OptionallyCoupledForce2.h"

registerMooseObject("MooseTestApp", OptionallyCoupledForce2);

template <>
InputParameters
validParams<OptionallyCoupledForce2>()
{
  InputParameters params = validParams<Kernel>();

  params.addCoupledVar("v", {1, 2}, "The two coupled variables which provide the force");

  return params;
}

OptionallyCoupledForce2::OptionallyCoupledForce2(const InputParameters & parameters)
  : Kernel(parameters)
{
  if (coupledComponents("v") != 2)
    mooseError("Should always have two coupled components.");
  _v_var.resize(2);
  _v.resize(2);
  for (unsigned int j = 0; j < 2; ++j)
  {
    _v_var[j] = coupled("v", j);
    _v[j] = &coupledValue("v", j);
  }
}

Real
OptionallyCoupledForce2::computeQpResidual()
{
  return -((*_v[0])[_qp] + (*_v[1])[_qp]) * _test[_i][_qp];
}

Real
OptionallyCoupledForce2::computeQpJacobian()
{
  return 0;
}
