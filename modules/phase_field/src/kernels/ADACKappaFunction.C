//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADACKappaFunction.h"

registerMooseObject("PhaseFieldApp", ADACKappaFunction);

InputParameters
ADACKappaFunction::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription("Gradient energy term for when kappa as a function of the variable");
  params.addParam<MaterialPropertyName>("mob_name", "L", "The mobility used with the kernel");
  params.addParam<MaterialPropertyName>("kappa_name", "kappa_op", "The kappa function name");
  params.addCoupledVar("v", "Vector of order parameters");
  return params;
}

ADACKappaFunction::ADACKappaFunction(const InputParameters & parameters)
  : ADKernel(parameters),
    _L(getADMaterialProperty<Real>("mob_name")),
    _kappa_name(getParam<MaterialPropertyName>("kappa_name")),
    _dkappadvar(getADMaterialProperty<Real>(derivativePropertyNameFirst(_kappa_name, _var.name()))),
    _v_num(coupledComponents("v")),
    _grad_eta(_v_num)
{
  for (unsigned int i = 0; i < _v_num; ++i)
    _grad_eta[i] = &adCoupledGradient("v", i);
}

ADReal
ADACKappaFunction::computeQpResidual()
{
  ADReal SumSquareGradOp = _grad_u[_qp] * _grad_u[_qp];
  for (unsigned int i = 0; i < _v_num; ++i)
    SumSquareGradOp += (*_grad_eta[i])[_qp] * (*_grad_eta[i])[_qp];

  return 0.5 * _test[_i][_qp] * _L[_qp] * _dkappadvar[_qp] * SumSquareGradOp;
}
