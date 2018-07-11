//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ACKappaFunction.h"

registerMooseObject("PhaseFieldApp", ACKappaFunction);

template <>
InputParameters
validParams<ACKappaFunction>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Gradient energy term for when kappa as a function of the variable");
  params.addParam<MaterialPropertyName>("mob_name", "L", "The mobility used with the kernel");
  params.addParam<MaterialPropertyName>("kappa_name", "kappa_op", "The kappa function name");
  params.addCoupledVar("v",
                       "Vector of additional nonlinear variables that affect the gradient energy");
  return params;
}

ACKappaFunction::ACKappaFunction(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>(parameters),
    _var_name(getParam<NonlinearVariableName>("variable")),
    _L_name(getParam<MaterialPropertyName>("mob_name")),
    _L(getMaterialProperty<Real>(_L_name)),
    _dLdvar(getMaterialPropertyDerivative<Real>(_L_name, _var_name)),
    _kappa_name(getParam<MaterialPropertyName>("kappa_name")),
    _dkappadvar(getMaterialPropertyDerivative<Real>(_kappa_name, _var_name)),
    _d2kappadvar2(getMaterialPropertyDerivative<Real>(_kappa_name, _var_name, _var_name)),
    _op_num(coupledComponents("v")),
    _v_name(_op_num),
    _grad_v(_op_num),
    _dLdv(_op_num),
    _d2kappadvardv(_op_num)
{
  for (unsigned int i = 0; i < _op_num; ++i)
  {
    _v_name[i] = getVar("v", i)->name();
    _grad_v[i] = &coupledGradient("v", i);
    _dLdv[i] = &getMaterialPropertyDerivative<Real>(_L_name, _v_name[i]);
    _d2kappadvardv[i] = &getMaterialPropertyDerivative<Real>(_kappa_name, _var_name, _v_name[i]);
  }
}

Real
ACKappaFunction::computeQpResidual()
{
  return 0.5 * _test[_i][_qp] * _L[_qp] * _dkappadvar[_qp] * computeFg();
}

Real
ACKappaFunction::computeQpJacobian()
{
  Real pre_jac = 0.5 * _test[_i][_qp] * _phi[_j][_qp] * computeFg();
  Real term1 = _test[_i][_qp] * _L[_qp] * _dkappadvar[_qp] * _grad_u[_qp] * _grad_phi[_j][_qp];

  return pre_jac * (_dLdvar[_qp] * _dkappadvar[_qp] + _L[_qp] * _d2kappadvar2[_qp]) + term1;
}

Real
ACKappaFunction::computeQpOffDiagJacobian(unsigned int jvar)
{
  const unsigned int i = mapJvarToCvar(jvar);
  Real pre_jac = 0.5 * _test[_i][_qp] * _phi[_j][_qp] * computeFg();
  Real term1 =
      _test[_i][_qp] * _L[_qp] * _dkappadvar[_qp] * (*_grad_v[i])[_qp] * _grad_phi[_j][_qp];
  return pre_jac * ((*_dLdv[i])[_qp] * _dkappadvar[_qp] + _L[_qp] * (*_d2kappadvardv[i])[_qp]) +
         term1;
}

Real
ACKappaFunction::computeFg()
{
  Real sum_grad_etai2 = 0.0;
  for (unsigned int i = 0; i < _op_num; ++i)
    sum_grad_etai2 += (*_grad_v[i])[_qp] * (*_grad_v[i])[_qp];

  Real grad_var2 = _grad_u[_qp] * _grad_u[_qp];
  return sum_grad_etai2 + grad_var2;
}
