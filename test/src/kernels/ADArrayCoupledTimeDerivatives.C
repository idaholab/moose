//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADArrayCoupledTimeDerivatives.h"

registerMooseObject("MooseTestApp", ADArrayCoupledTimeDerivatives);

InputParameters
ADArrayCoupledTimeDerivatives::validParams()
{
  InputParameters params = ADArrayKernel::validParams();
  params.addCoupledVar("v", "The coupled array variable; the kernel variable if not given");
  params.addParam<Real>("a", 1.0, "Coefficient of the first time derivative");
  params.addParam<Real>("b", 0.0, "Coefficient of the second time derivative");
  params.addClassDescription("Adds the residual $(a \\dot{\\vec{v}} + b \\ddot{\\vec{v}}, "
                             "\\vec{u}^\\ast)$ using automatic differentiation.");
  return params;
}

ADArrayCoupledTimeDerivatives::ADArrayCoupledTimeDerivatives(const InputParameters & parameters)
  : ADArrayKernel(parameters),
    _dot(getParam<Real>("a") == 0 ? nullptr
                                  : (isCoupled("v") ? &adCoupledArrayDot("v") : &_var.adUDot())),
    _dotdot(isCoupled("v") ? adCoupledArrayDotDot("v") : _var.adUDotDot()),
    _a(getParam<Real>("a")),
    _b(getParam<Real>("b"))
{
}

void
ADArrayCoupledTimeDerivatives::computeQpResidual(ADRealEigenVector & residual)
{
  residual = _b * _dotdot[_qp] * _test[_i][_qp];
  if (_dot)
    residual += _a * (*_dot)[_qp] * _test[_i][_qp];
}
