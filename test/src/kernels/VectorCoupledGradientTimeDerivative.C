//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorCoupledGradientTimeDerivative.h"
#include "Function.h"
#include "Assembly.h"

registerMooseObject("MooseTestApp", VectorCoupledGradientTimeDerivative);

InputParameters
VectorCoupledGradientTimeDerivative::validParams()
{
  InputParameters params = VectorKernel::validParams();
  params.addRequiredCoupledVar("v",
                               "The standard MooseVariable whose time derivative of the gradient "
                               "we will add to the residual. A physical realization of this is in "
                               "the Coulomb gauge formulation of Maxwells' equations.");
  return params;
}

VectorCoupledGradientTimeDerivative::VectorCoupledGradientTimeDerivative(
    const InputParameters & parameters)
  : VectorKernel(parameters),
    _grad_v_dot(coupledGradientDot("v")),
    _d_grad_v_dot_dv(coupledDotDu("v")),
    _v_id(coupled("v")),
    _v_var(*getVar("v", 0)),
    _standard_grad_phi(_assembly.gradPhi(_v_var))
{
}

Real
VectorCoupledGradientTimeDerivative::computeQpResidual()
{
  return _test[_i][_qp] * _grad_v_dot[_qp];
}

Real
VectorCoupledGradientTimeDerivative::computeQpJacobian()
{
  return 0;
}

Real
VectorCoupledGradientTimeDerivative::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _v_id)
    return _test[_i][_qp] * _d_grad_v_dot_dv[_qp] * _standard_grad_phi[_j][_qp];

  else
    return 0.;
}
