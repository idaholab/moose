//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OptionallyCoupledForce.h"

registerMooseObject("MooseTestApp", OptionallyCoupledForce);

InputParameters
OptionallyCoupledForce::validParams()
{
  InputParameters params = Kernel::validParams();

  params.addCoupledVar("v", 1, "The coupled variable which provides the force");

  return params;
}

OptionallyCoupledForce::OptionallyCoupledForce(const InputParameters & parameters)
  : Kernel(parameters),
    _v_var(coupled("v")),
    _v(coupledValue("v")),
    _grad_v(coupledGradient("v")),
    _second_v(coupledSecond("v")),
    _v_dot(_is_transient ? coupledDot("v") : _zero),
    _v_dot_du(_is_transient ? coupledDotDu("v") : _zero),
    _v_coupled(isCoupled("v"))
{
  if (!_v_coupled && _v_var < 64)
    mooseError("Something is wrong with the coupling system.  It should be producing really huge "
               "numbers for coupled('v') But instead it generated: ",
               _v_var);
}

Real
OptionallyCoupledForce::computeQpResidual()
{
  return -_v[_qp] * _test[_i][_qp];
}

Real
OptionallyCoupledForce::computeQpJacobian()
{
  return 0;
}

Real
OptionallyCoupledForce::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _v_var)
    return -_phi[_j][_qp] * _test[_i][_qp];
  return 0.0;
}
