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

registerMooseObject("MooseTestApp", VectorCoupledGradientTimeDerivative);

template <>
InputParameters
validParams<VectorCoupledGradientTimeDerivative>()
{
  InputParameters params = validParams<VectorKernel>();
  params.addRequiredCoupledVar("V",
                               "The standard MooseVariable whose time derivative of the gradient "
                               "we will add to the residual. A physical realization of this is in "
                               "the Coulomb gauge formulation of Maxwells' equations.");
  return params;
}

VectorCoupledGradientTimeDerivative::VectorCoupledGradientTimeDerivative(
    const InputParameters & parameters)
  : VectorKernel(parameters),
    _grad_V_dot(coupledGradientDot("V")),
    _d_grad_V_dot_dV(coupledDotDu("V")),
    _V_id(coupled("V")),
    _V_var(*getVar("V", 0)),
    _V_grad_phi(_assembly.gradPhi(_V_var))
{
}

Real
VectorCoupledGradientTimeDerivative::computeQpResidual()
{
  return _test[_i][_qp] * _grad_V_dot[_qp];
}

Real
VectorCoupledGradientTimeDerivative::computeQpJacobian()
{
  return 0;
}

Real
VectorCoupledGradientTimeDerivative::computeQpOffDiagJacobian(unsigned jvar)
{
  if (jvar == _V_id)
    return _test[_i][_qp] * _d_grad_V_dot_dV[_qp] * _V_grad_phi[_j][_qp];

  else
    return 0.;
}
