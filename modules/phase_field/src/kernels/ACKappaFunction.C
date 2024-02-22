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
registerMooseObject("PhaseFieldApp", ADACKappaFunction);

template <bool is_ad>
InputParameters
ACKappaFunctionTempl<is_ad>::validParams()
{
  InputParameters params = GenericKernel<is_ad>::validParams();
  params.addClassDescription("Gradient energy term for when kappa as a function of the variable");
  params.addParam<MaterialPropertyName>("mob_name", "L", "The mobility used with the kernel");
  params.addParam<MaterialPropertyName>("kappa_name", "kappa_op", "The kappa function name");
  params.addCoupledVar("v", "Vector of order parameters");
  return params;
}
template <bool is_ad>
ACKappaFunctionTempl<is_ad>::ACKappaFunctionTempl(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<GenericKernel<is_ad>>>(parameters),
    _L(this->template getGenericMaterialProperty<Real, is_ad>("mob_name")),
    _kappa_name(this->template getParam<MaterialPropertyName>("kappa_name")),
    _dkappadvar(this->template getGenericMaterialProperty<Real, is_ad>(
        this->derivativePropertyNameFirst(_kappa_name, _var.name()))),
    _v_num(this->coupledComponents("v")),
    _grad_v(_v_num)
{
  for (unsigned int i = 0; i < _v_num; ++i)
    _grad_v[i] = &this->template coupledGenericGradient<is_ad>("v", i);
}

ACKappaFunction::ACKappaFunction(const InputParameters & parameters)
  : ACKappaFunctionTempl<false>(parameters),
    _dLdvar(getMaterialPropertyDerivative<Real>("mob_name", _var.name())),
    _d2kappadvar2(getMaterialPropertyDerivative<Real>(_kappa_name, _var.name(), _var.name())),
    _v_map(getParameterJvarMap("v")),
    _dLdv(_v_num),
    _d2kappadvardv(_v_num)
{
  for (unsigned int i = 0; i < _v_num; ++i)
  {
    auto v_name = coupledName("v", i);
    _dLdv[i] = &getMaterialPropertyDerivative<Real>("mob_name", v_name);
    _d2kappadvardv[i] = &getMaterialPropertyDerivative<Real>(_kappa_name, _var.name(), v_name);
  }
}

template <bool is_ad>
GenericReal<is_ad>
ACKappaFunctionTempl<is_ad>::computeQpResidual()
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
  auto i = mapJvarToCvar(jvar, _v_map);
  if (i >= 0)
  {
    const Real pre_jac = 0.5 * _test[_i][_qp] * _phi[_j][_qp] * computeFg();
    const Real term1 =
        _test[_i][_qp] * _L[_qp] * _dkappadvar[_qp] * (*_grad_v[i])[_qp] * _grad_phi[_j][_qp];
    return pre_jac * ((*_dLdv[i])[_qp] * _dkappadvar[_qp] + _L[_qp] * (*_d2kappadvardv[i])[_qp]) +
           term1;
  }

  return 0.0;
}

template <bool is_ad>
GenericReal<is_ad>
ACKappaFunctionTempl<is_ad>::computeFg()
{
  GenericReal<is_ad> sum_grad_etai2 = 0.0;
  for (unsigned int i = 0; i < _v_num; ++i)
    sum_grad_etai2 += (*_grad_v[i])[_qp] * (*_grad_v[i])[_qp];

  return sum_grad_etai2 + _grad_u[_qp] * _grad_u[_qp];
}
